#include "zygisk.hpp"
#include "touch.h"
#include "draw.h"
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>

#define LOG_TAG "ZygiskModule"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalMenuModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *process_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (process_name) {
            LOGI("preAppSpecialize target: %s", process_name);
            // Ignore system processes, target application processes
            if (strstr(process_name, "com.") != nullptr && strstr(process_name, "android.") == nullptr) {
                is_target_app = true;
            }
            env->ReleaseStringUTFChars(*args->nice_name, process_name);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (!is_target_app) return;

        LOGI("postAppSpecialize: Initializing graphics and touch hooks");
        pthread_t thread;
        pthread_create(&thread, nullptr, [](void *) -> void * {
            sleep(1); // Allow target process graphics context to initialize
            TouchHook::Init();
            GraphicsHook::Init();
            return nullptr;
        }, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool is_target_app = false;
};

static void companion_handler(int socket) {
    // Companion process socket handler for Magisk v24-26
}

REGISTER_ZYGISK_MODULE(UniversalMenuModule)
REGISTER_ZYGISK_COMPANION(companion_handler)

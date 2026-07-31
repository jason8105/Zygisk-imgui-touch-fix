#include <jni.h>
#include <android/log.h>
#include <thread>
#include <unistd.h>
#include <cstring>
#include "zygisk.hpp"
#include "hooks/graphics_hook.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class ImGuiUniversalModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *process_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process_name) {
            // Target app process filtering (universal for user applications)
            if (is_target_application(process_name)) {
                is_target = true;
                LOGI("Target process identified: %s", process_name);
            }
            env->ReleaseStringUTFChars(args->nice_name, process_name);
        }

        if (!is_target) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_FOR_UNNEEDED_PROCESS);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (is_target) {
            std::thread(init_thread).detach();
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool is_target = false;

    static bool is_target_application(const char *name) {
        if (!name) return false;
        // Exclude Android system processes and Zygote services
        if (strstr(name, "com.android.") ||
            strstr(name, "system_server") ||
            strstr(name, "android.process.") ||
            strstr(name, "com.google.android.")) {
            return false;
        }
        return true;
    }

    static void init_thread() {
        sleep(1);
        GraphicsHook::Init();
    }
};

REGISTER_ZYGISK_MODULE(ImGuiUniversalModule)

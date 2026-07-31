#include "zygisk.hpp"
#include "hooks/hooks.h"
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <string>

#define LOG_TAG "ZygiskMain"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static void* InitThread(void*) {
    sleep(1);
    Hooks::Init();
    return nullptr;
}

class UniversalModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *nice_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (nice_name) {
            std::string process_name(nice_name);
            env->ReleaseStringUTFChars(*args->nice_name, nice_name);

            if (!process_name.empty() && 
                process_name.find("com.android.") == std::string::npos && 
                process_name.find("system_server") == std::string::npos) {
                is_target = true;
            }
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (is_target) {
            pthread_t thread;
            pthread_create(&thread, nullptr, InitThread, nullptr);
            pthread_detach(thread);
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool is_target = false;
};

REGISTER_ZYGISK_MODULE(UniversalModule)

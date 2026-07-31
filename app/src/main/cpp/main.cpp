#include "zygisk.hpp"
#include "hook_engine.h"
#include <android/log.h>
#include <cstring>

#define LOG_TAG "UniversalZygisk"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *nice_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (nice_name) {
            LOGI("Zygisk target process fork: %s", nice_name);
            if (shouldInject(nice_name)) {
                LOGI("Injecting universal ImGui and touch hooks into process: %s", nice_name);
                HookEngine::InstallHooks();
            }
            env->ReleaseStringUTFChars(*args->nice_name, nice_name);
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;

    bool shouldInject(const char* process_name) {
        if (!process_name) return false;
        // Ignore core Android system services and systemui
        if (strstr(process_name, "com.android.") != nullptr ||
            strstr(process_name, "system_server") != nullptr ||
            strstr(process_name, "com.google.android.") != nullptr) {
            return false;
        }
        return true;
    }
};

REGISTER_ZYGISK_MODULE(UniversalZygiskModule)

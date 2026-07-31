#include "zygisk.hpp"
#include "graphics/graphics_hook.h"
#include "touch/touch_hook.h"
#include <android/log.h>
#include <string.h>

#define LOG_TAG "ZygiskImGuiModule"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *process_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (process_name) {
            LOGI("Zygisk injected into process: %s", process_name);

            // Install graphics and touch hooks
            GraphicsHook::InstallHooks();
            TouchHook::InstallHooks();

            env->ReleaseStringUTFChars(*args->nice_name, process_name);
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

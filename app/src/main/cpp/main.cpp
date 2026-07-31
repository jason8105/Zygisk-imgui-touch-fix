#include "zygisk.hpp"
#include "imgui_manager.h"
#include "touch_hook.h"
#include <android/log.h>

#define LOG_TAG "ZygiskUniversalMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *process_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (process_name) {
            should_inject = (process_name[0] != '\0');
            env->ReleaseStringUTFChars(*args->nice_name, process_name);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (should_inject) {
            LOGI("Specialized target process - loading touch hook and ImGui context");
            TouchHook::Init(env);
            ImGuiManager::Init(env);
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool should_inject = false;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

#include <zygisk.hpp>
#include <android/log.h>
#include <string>
#include "hooks/input_hook.h"
#include "hooks/egl_hook.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *process_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process_name) {
            std::string name(process_name);
            env->ReleaseStringUTFChars(args->nice_name, process_name);

            // Ignore isolated/system zygote processes
            if (name.find("com.android.") == std::string::npos &&
                name.find("system_ui") == std::string::npos &&
                name.find("com.google.") == std::string::npos) {
                is_target = true;
                LOGI("Target application detected: %s", name.c_str());
            }
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (is_target) {
            api->setOption(zygisk::DLCLOSE_MODULE_FOR_UNLOADED_PROCESS);
            init_universal_input_hooks(env);
            init_universal_egl_hooks(env);
            LOGI("Hooks injected into target process.");
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool is_target = false;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

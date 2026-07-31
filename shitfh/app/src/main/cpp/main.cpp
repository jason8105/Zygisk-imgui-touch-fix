#include <jni.h>
#include <android/log.h>
#include <string>
#include "zygisk.hpp"
#include "touch.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class ImGuiZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) return;

        const char* raw_process_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (raw_process_name) {
            process_name = raw_process_name;
            env->ReleaseStringUTFChars(*args->nice_name, raw_process_name);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (process_name.empty()) return;

        // Ignore system processes and zygote
        if (process_name.find("com.android.") == 0 || process_name.find("system_server") != std::string::npos) {
            api->setOption(api, zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        LOGI("Injected into target process: %s", process_name.c_str());
        TouchHandler::InitHooks();
    }

private:
    zygisk::Api* api = nullptr;
    JNIEnv* env = nullptr;
    std::string process_name;
};

REGISTER_ZYGISK_MODULE(ImGuiZygiskModule)

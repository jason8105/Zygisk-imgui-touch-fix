#include "zygisk.hpp"
#include "hook.h"
#include <jni.h>
#include <android/log.h>
#include <string.h>

#define LOG_TAG "ZygiskImGuiUniversal"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecimen *specimen) override {
        const char *process_name = specimen->getProcessName();
        if (process_name == nullptr) return;

        // Skip isolated system components
        if (strstr(process_name, "com.android") == nullptr &&
            strstr(process_name, "system") == nullptr) {
            LOGI("Target application detected: %s. Installing universal hooks...", process_name);
            setup_hooks();
        } else {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

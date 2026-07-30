#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include "zygisk.hpp"
#include "input_hook.h"
#include "imgui_impl.h"

#define LOG_TAG "ZygiskMain"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class ZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
        LOGI("ZygiskModule onLoad loaded successfully!");
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Intercept app specialization for Magisk v24-26 compatibility
        InputHook::initHooks();
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        LOGI("ZygiskModule postAppSpecialize executed.");
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

static LibraryLoader g_loader;
REGISTER_ZYGISK_MODULE(ZygiskModule)

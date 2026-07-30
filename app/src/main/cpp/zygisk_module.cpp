#include <jni.h>
#include <android/log.h>
#include "zygisk.hpp"

#define LOG_TAG "ZygiskImGuiModule"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern void InitializeUniversalTouchHooks();

class ImGuiZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
        LOGI("[+] ImGuiZygiskModule loaded into process.");
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Safe to hook or initialize before app process specialization
        InitializeUniversalTouchHooks();
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        LOGI("[+] App specialized successfully.");
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

// Register Zygisk module entry point compatible with Magisk v24-26
ZYGISK_MODULE_ENTRY(ImGuiZygiskModule)

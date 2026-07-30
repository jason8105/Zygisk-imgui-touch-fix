#include "zygisk.hpp"
#include <jni.h>
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void initializeImGuiHooks();

class ImGuiZygiskModule : public zygisk::ZygiskModule {
public:
    void onPreSpecialize(zygisk::AppSpecializeArgs *args) override {
        LOGI("onPreSpecialize invoked for package: %s", args->nice_name ? args->nice_name : "unknown");
    }

    void onPostSpecialize(zygisk::AppSpecializeArgs *args) override {
        LOGI("onPostSpecialize invoked, initializing ImGui hooks");
        initializeImGuiHooks();
    }
};

static zygisk::LoaderAPI *g_api = nullptr;

extern "C" {

void zygisk_module_entry(zygisk::LoaderAPI *api, JNIEnv *env) {
    g_api = api;
    LOGI("zygisk_module_entry loaded successfully for Magisk v24-26 compatible environment.");
}

bool register_zygisk_module(zygisk::LoaderAPI *api) {
    auto module = new ImGuiZygiskModule();
    return api->registerModule(api, module);
}

} // extern "C"

// Magisk Zygisk entry point macro mapping
__attribute__((constructor)) static void init() {
    // Constructor registration for Zygisk v24-26
    LOGI("Zygisk ImGui native library loaded into zygote.");
}

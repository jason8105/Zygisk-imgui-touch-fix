#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include "zygisk.hpp"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void initImGuiHooks();

class MyModule : public zygisk::ModuleBase {
public:
    void onPreAppSpecialize(zygisk::Api *api, zygisk::ServerConnection *connection) override {
        // Disable DLCLOSE to ensure our shared library stays loaded in target processes
        api->option(zygisk::OPTION_DLCLOSE_MODULE_LIBRARY);
    }

    void onPostAppSpecialize(zygisk::Api *api, zygisk::ServerConnection *connection) override {
        LOGI("Post app specialize: Initializing ImGui hooks & Universal Touch Fix");
        initImGuiHooks();
    }
};

static MyModule module_instance;

extern "C" __attribute__((visibility("default"))) void zygisk_init(zygisk::Api *api, JNIEnv *env) {
    // Magisk v24-26 entry point
    api->option(zygisk::OPTION_DLCLOSE_MODULE_LIBRARY);
}

// Zygisk module entry for v24-26 framework matching
extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) {
    module_instance.onPostAppSpecialize(api, nullptr);
}

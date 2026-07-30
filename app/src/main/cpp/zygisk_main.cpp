#include <jni.h>
#include <android/log.h>
#include "zygisk.hpp"

#define LOG_TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

void initImGuiHooks();

class MyModule : public zygisk::ZygiskModule {
public:
    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Not used
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        LOGD("postAppSpecialize triggered, initializing ImGui and universal touch hooks...");
        initImGuiHooks();
    }
};

static zygisk::ZygiskApi *g_api = nullptr;

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void zygisk_module_entry(zygisk::ZygiskApi *api, int version) {
    g_api = api;
    static MyModule module;
}

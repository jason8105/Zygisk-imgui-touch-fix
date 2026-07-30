#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include "zygisk.hpp"

#define LOG_TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

extern void init_imgui_hooks();

class ZygiskImGuiModule : public zygisk::ModuleBase {
public:
    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (args && args->package_name) {
            LOGD("Loading into package: %s", args->package_name);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        static_cast<void>(args);
        LOGD("Initializing universal ImGui touch hooks post specialize...");
        init_imgui_hooks();
    }
};

REGISTER_ZYGISK_MODULE(ZygiskImGuiModule)

#include "zygisk.hpp"
#include "touch_hook.hpp"
#include "imgui_impl.hpp"
#include <android/log.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include <GLES3/gl3.h>

#define LOG_TAG "ZygiskZygote"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

class ZygiskImGuiModule : public zygisk::ModuleBase {
public:
    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Intercept app specialization in Zygisk v24-26
        LOGD("preAppSpecialize invoked");
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        LOGD("postAppSpecialize invoked - initializing ImGui and Touch Hooks");
        
        // Initialize ImGui and Universal Touch Hook
        ImGuiImpl::init();
        TouchHook::initHooks();
    }
};

REGISTER_ZYGISK_MODULE(ZygiskImGuiModule)

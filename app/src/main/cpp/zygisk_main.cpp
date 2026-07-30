#include <jni.h>
#include <android/log.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include "zygisk.hpp"
#include "imgui_integration.h"

using zygisk::Api;
using zygisk::AppSpecArgs;
using zygisk::ServerSpecArgs;

class ZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
        LOGD("ZygiskModule onLoad loaded into process");
    }

    void preAppSpecialize(AppSpecArgs* args) override {
        // Optional pre-specialize hooks
    }

    void postAppSpecialize(AppSpecArgs* args) override {
        LOGD("ZygiskModule postAppSpecialize executed");
        // Initialize hooks or display integration here
        // For demonstration, hook EGL swap buffers or input dispatch if desired
    }

private:
    Api* api;
    JNIEnv* env;
};

// Register Zygisk module entry point compatible with Magisk v24-26
ZYGISK_MODULE_ENTRY(ZygiskModule)

// Optional hook interceptor for universal touch dispatch via AInputQueue or native APIs
extern "C" __attribute__((visibility("default")))
bool NativeHandleInput(AInputEvent* event) {
    return ImGuiIntegration::HandleInput(event);
}

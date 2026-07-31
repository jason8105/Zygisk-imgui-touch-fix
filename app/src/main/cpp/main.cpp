#include "zygisk.hpp"
#include "render_hook.h"
#include "touch_hook.h"
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (args->is_top_app && *args->is_top_app) {
            LOGI("Universal ImGui Module loaded into top app");
            RenderHook::Init();
            TouchHook::Init();
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalModule)

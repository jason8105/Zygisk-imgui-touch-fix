#include <jni.h>
#include <thread>
#include <chrono>
#include <android/log.h>
#include "zygisk.hpp"
#include "hooks/touch_hook.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void init_hooks() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    LOGI("Initializing Zygisk ImGui universal touch and rendering hooks...");
    init_touch_hook();
}

class ImGuiZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *process_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process_name) {
            LOGI("App specializing process: %s", process_name);
            enable_hack = true;
            env->ReleaseStringUTFChars(args->nice_name, process_name);
        }

        if (enable_hack && api) {
            api->setOption(zygisk::DLCLOSE_MODULE_LOADED_LIB);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (enable_hack) {
            std::thread(init_hooks).detach();
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool enable_hack = false;
};

REGISTER_ZYGISK_MODULE(ImGuiZygiskModule)

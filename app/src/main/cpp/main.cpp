#include "zygisk.hpp"
#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include "hooks/input_hook.h"
#include "hooks/egl_hook.h"

#define LOG_TAG "ZygiskImGuiMain"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class ImGuiZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        pthread_t thread;
        pthread_create(&thread, nullptr, [](void *) -> void * {
            sleep(1);
            InputHook::Init();
            EGLHook::Init();
            LOGI("Zygisk ImGui Menu and Universal Input Hook initialized");
            return nullptr;
        }, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(ImGuiZygiskModule)

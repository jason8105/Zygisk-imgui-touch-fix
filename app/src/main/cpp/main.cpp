#include "zygisk.hpp"
#include "egl/egl_hook.h"
#include "touch/touch_hook.h"
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

#define LOG_TAG "ZygiskModule"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static void* InitThread(void*) {
    sleep(2);
    LOGI("Initializing Zygisk ImGui Universal Menu...");
    
    TouchHook::InstallHooks();
    EGLHook::InstallHooks();

    return nullptr;
}

class UniversalModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpec *spec) override {
        pthread_t thread;
        pthread_create(&thread, nullptr, InitThread, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalModule)

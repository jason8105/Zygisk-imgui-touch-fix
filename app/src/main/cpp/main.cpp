#include "zygisk.hpp"
#include "hooks/egl_hook.h"
#include "hooks/touch_hook.h"
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "ZygiskMain"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static void *init_thread(void *) {
    sleep(2);
    LOGI("Universal ImGui Module starting...");
    init_touch_hooks();
    init_egl_hooks();
    return nullptr;
}

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        pthread_t thread;
        pthread_create(&thread, nullptr, init_thread, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

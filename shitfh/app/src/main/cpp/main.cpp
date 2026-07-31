#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include "zygisk.hpp"
#include "touch_hook.h"

#define LOG_TAG "ZygiskModule"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalMenuModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        pthread_t thread;
        pthread_create(&thread, nullptr, [](void *) -> void * {
            usleep(1000000);
            for (int i = 0; i < 5; i++) {
                SetupTouchAndOverlayHooks();
                usleep(1000000);
            }
            return nullptr;
        }, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalMenuModule)

#include "zygisk.hpp"
#include "touch.h"
#include "gui.h"
#include <android/log.h>
#include <unistd.h>
#include <pthread.h>

#define LOG_TAG "ZygiskModule"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        pthread_t thread;
        pthread_create(&thread, nullptr, [](void *) -> void * {
            usleep(500000); // Wait for native libraries to finish loading
            UniversalTouch::Init();
            MenuGUI::Init();
            LOGI("Universal Zygisk ImGui Menu loaded into target app.");
            return nullptr;
        }, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

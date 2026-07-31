#include "zygisk.hpp"
#include "hooks/graphics_hook.hpp"
#include "hooks/touch_hook.hpp"
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static void *hook_thread(void *) {
    LOGI("Universal hook thread initialized in target process");
    
    sleep(1);

    int retries = 0;
    while (retries < 20) {
        bool gfx = GraphicsHook::Init();
        bool touch = TouchHook::Init();
        if (gfx || touch) {
            LOGI("Hooks applied successfully (Graphics: %d, Touch: %d)", gfx, touch);
            break;
        }
        usleep(500000);
        retries++;
    }

    return nullptr;
}

class ImGuiZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;
        api->setOption(zygisk::Option::DLCLOSE_MODULE_FOR_UNLOADED_PROCESS);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        pthread_t thread;
        pthread_create(&thread, nullptr, hook_thread, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(ImGuiZygiskModule)

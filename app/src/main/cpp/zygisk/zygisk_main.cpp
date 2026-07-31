#include "zygisk.hpp"
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include "../render/render_hook.h"
#include "../touch/touch_hook.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpec(zygisk::AppSpec *spec) override {
        if (!spec || !spec->nice_name) return;

        // Enable module for target process or all non-system user apps
        const char *process_name = spec->nice_name;
        if (spec->uid >= 10000 && process_name && std::strlen(process_name) > 0) {
            is_target = true;
            LOGI("Target application matched: %s (uid: %d)", process_name, spec->uid);
        }
    }

    void postAppSpec(zygisk::AppSpec *spec) override {
        if (!is_target) return;

        // Unmount denylist forces or unload if needed
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);

        pthread_t tid;
        pthread_create(&tid, nullptr, [](void *) -> void * {
            // Give game engine dynamic libraries time to load
            sleep(2);
            LOGI("Initializing Render & Universal Touch Hooks...");
            TouchHook::Init();
            RenderHook::Init();
            return nullptr;
        }, nullptr);
        pthread_detach(tid);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool is_target = false;
};

REGISTER_ZYGISK_MODULE(UniversalZygiskModule)

#include "zygisk.hpp"
#include "touch_hook.hpp"
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

class ZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        pthread_t thread;
        pthread_create(&thread, nullptr, [](void *) -> void * {
            usleep(1500000);
            TouchHook::Init();
            return nullptr;
        }, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(ZygiskModule)

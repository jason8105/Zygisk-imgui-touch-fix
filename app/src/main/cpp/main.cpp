#include "zygisk.hpp"
#include "hooks.hpp"
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

#define LOG_TAG "ZygiskImGuiMain"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *pkg_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (pkg_name) {
            LOGI("Injected into process: %s", pkg_name);
            pthread_t thread;
            pthread_create(&thread, nullptr, [](void *) -> void * {
                sleep(1);
                Hooks::Init();
                return nullptr;
            }, nullptr);
            pthread_detach(thread);

            env->ReleaseStringUTFChars(*args->nice_name, pkg_name);
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

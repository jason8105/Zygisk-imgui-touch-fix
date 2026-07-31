#include "zygisk.hpp"
#include "menu/menu.h"
#include <thread>
#include <chrono>
#include <cstring>
#include <android/log.h>

#define LOG_TAG "ZygiskMain"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;
        
        const char *proc_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (proc_name) {
            // Filter out core system apps, target general user apps/games
            if (strstr(proc_name, "com.android") == nullptr &&
                strstr(proc_name, "system_server") == nullptr &&
                strstr(proc_name, "com.google") == nullptr) {
                target_app = true;
                LOGI("Target application detected: %s", proc_name);
            }
            env->ReleaseStringUTFChars(args->nice_name, proc_name);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (target_app) {
            api->setOption(zygisk::DLCLOSE_MODULE_AT_UNLOAD);
            std::thread([]() {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                InitMenuAndHooks();
            }).detach();
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool target_app = false;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

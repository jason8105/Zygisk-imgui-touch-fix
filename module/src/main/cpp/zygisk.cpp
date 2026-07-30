#include "zygisk.hpp"
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void init_imgui_hooks(zygisk::Api* api);

class MyZygiskModule : public zygisk::Module {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        LOGI("MyZygiskModule onLoad called for Magisk 24-26");
    }

    void preAppSpecialize(zygisk::Api *api, JNIEnv *env, const char *app_data_dir, const char *process_name) override {
        // Apply hooks targeting apps/games
        init_imgui_hooks(api);
    }

    void postAppSpecialize(zygisk::Api *api, JNIEnv *env, const char *app_data_dir, const char *process_name) override {
        // Post specialization tasks if needed
    }
};

REGISTER_ZYGISK_MODULE(MyZygiskModule)

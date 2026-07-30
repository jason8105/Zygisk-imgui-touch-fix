#include <jni.h>
#include <android/log.h>
#include "zygisk.hpp"
#include "hooks.h"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZygiskImGui", __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void onPostForkInChild(const AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process && strstr(process, "com.target.game")) { // Replace with targeting logic
            LOGD("Injecting into: %s", process);
            install_universal_hooks();
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

private:
    Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(MyModule)

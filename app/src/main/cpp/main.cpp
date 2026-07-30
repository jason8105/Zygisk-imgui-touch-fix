#include <jni.h>
#include <string>
#include "zygisk.hpp"
#include "hooks.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process && strstr(process, "com.your.target.game")) { // Replace or make configurable
            enable_hooks = true;
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        if (enable_hooks) {
            install_hooks();
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool enable_hooks = false;
};

REGISTER_ZYGISK_MODULE(MyModule)

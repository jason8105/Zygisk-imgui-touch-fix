#include <jni.h>
#include "zygisk.hpp"
#include "input_hook.h"
#include <android/log.h>

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
        if (process) {
            // Filter your target package here or apply to all
            this->is_target = true; 
            env->ReleaseStringUTFChars(args->nice_name, process);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (this->is_target) {
            // Standard Magisk 24-26 way to initialize hooks in the app process
            install_input_hooks();
            // Note: ImGui initialization should happen where EGL/GLES is initialized (e.g. hook eglSwapBuffers)
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool is_target = false;
};

REGISTER_ZYGISK_MODULE(MyModule)

#include <unistd.h>
#include <thread>
#include <string>
#include <dlfcn.h>
#include <android/log.h>
#include "zygisk.hpp"
#include "hook.h"
#include "touch.h"
#include "menu.h"

#define LOG_TAG "ZygiskImGuiMain"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static void install_all_hooks() {
    sleep(1);

    void* libegl = dlopen("libEGL.so", RTLD_NOW);
    void* orig_swap = nullptr;
    if (libegl) {
        orig_swap = dlsym(libegl, "eglSwapBuffers");
    }

    hook_plt_all("eglSwapBuffers", (void*)hook_eglSwapBuffers, &orig_swap);
    install_touch_hooks();

    LOGI("Universal ImGui and Touch Hooks installed successfully.");
}

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecimen *specimen) override {
        if (!specimen || !specimen->process_name) return;

        std::string process_name = specimen->process_name;

        if (process_name.find("com.android.") == 0 ||
            process_name.find("system_server") != std::string::npos ||
            process_name.find("android.process.") == 0) {
            return;
        }

        LOGI("Target application specialized: %s", process_name.c_str());
        std::thread(install_all_hooks).detach();
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

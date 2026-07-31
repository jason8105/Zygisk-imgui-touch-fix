#include "zygisk.hpp"
#include "plt_hook.h"
#include "touch_hook.h"
#include "imgui_manager.h"
#include <EGL/egl.h>
#include <android/log.h>
#include <thread>
#include <chrono>

#define LOG_TAG "ZygiskMain"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    ImGuiManager::Render(dpy, surface);
    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_FALSE;
}

static void HookLoop() {
    bool hooked_egl = false;
    for (int i = 0; i < 50; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (!hooked_egl) {
            hooked_egl = PltHook::HookAll("eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            if (hooked_egl) {
                LOGI("Successfully hooked eglSwapBuffers");
            }
        }

        TouchHook::Init();

        if (hooked_egl) break;
    }
}

class UniversalModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        std::thread(HookLoop).detach();
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalModule)

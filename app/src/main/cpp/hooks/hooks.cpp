#include "hooks.h"
#include "../gui/gui.h"
#include "../touch/touch_hook.h"
#include "../dobby/dobby.h"
#include <EGL/egl.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "ZygiskHooks"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    static bool guiInitialized = false;
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (!guiInitialized && width > 0 && height > 0) {
        GUI::Init(width, height);
        guiInitialized = true;
        LOGI("GUI successfully initialized: %dx%d", width, height);
    } else if (guiInitialized && width > 0 && height > 0) {
        GUI::UpdateDisplaySize(width, height);
    }

    GUI::Render();

    return orig_eglSwapBuffers(dpy, surface);
}

namespace Hooks {
void Init() {
    void* libegl = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    if (libegl) {
        void* swapBuffers_sym = dlsym(libegl, "eglSwapBuffers");
        if (swapBuffers_sym) {
            DobbyHook(swapBuffers_sym, (dobby_dummy_func_t)hook_eglSwapBuffers, (dobby_dummy_func_t*)&orig_eglSwapBuffers);
            LOGI("Hooked eglSwapBuffers successfully");
        }
    }

    TouchHook::Init();
}
}

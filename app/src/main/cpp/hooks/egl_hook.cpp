#include "egl_hook.h"
#include "hook_utils.h"
#include "../gui/menu.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "ZygiskEGL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    static bool menu_initialized = false;
    if (!menu_initialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        if (width > 0 && height > 0) {
            init_menu(width, height);
            menu_initialized = true;
        }
    }

    if (menu_initialized) {
        render_menu();
    }

    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

void init_egl_hooks() {
    LOGI("Initializing EGL Hooks...");
    void* libegl = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    if (libegl) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(libegl, "eglSwapBuffers");
    }
    hook_plt(nullptr, "eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
}

#include "hook.h"
#include "touch.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>
#include <link.h>
#include <string.h>

#define LOG_TAG "UniversalHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace Hook {

void PLTHook(const char* libName, const char* symName, void* newFunc, void** oldFunc) {
    void* handle = dlopen(libName, RTLD_NOW);
    if (!handle) return;
    
    void* sym = dlsym(handle, symName);
    if (sym && oldFunc && !*oldFunc) {
        *oldFunc = sym;
    }
    dlclose(handle);
}

void InitHooks(void* swapBuffersHook, void** origSwapBuffers) {
    // Hook EGL swap buffers
    PLTHook("libEGL.so", "eglSwapBuffers", swapBuffersHook, origSwapBuffers);

    // Universal Touch Hooking for libandroid.so AInputQueue
    void* getEventSym = nullptr;
    void* finishEventSym = nullptr;
    
    PLTHook("libandroid.so", "AInputQueue_getEvent", (void*)Touch::Hook_AInputQueue_getEvent, &getEventSym);
    PLTHook("libandroid.so", "AInputQueue_finishEvent", nullptr, &finishEventSym);

    if (getEventSym) Touch::SetOriginalGetEvent(getEventSym);
    if (finishEventSym) Touch::SetOriginalFinishEvent(finishEventSym);

    Touch::Init();
    LOGI("Universal hooks initialized.");
}

} // namespace Hook

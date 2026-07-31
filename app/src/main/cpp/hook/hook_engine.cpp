#include "hook_engine.h"
#include "menu.h"
#include <dlfcn.h>
#include <EGL/egl.h>
#include <android/input.h>
#include <android/log.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

#define LOG_TAG "UniversalHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
typedef int32_t (*AInputEvent_getType_t)(const AInputEvent* event);
typedef int32_t (*AMotionEvent_getAction_t)(const AInputEvent* motion_event);
typedef float (*AMotionEvent_getX_t)(const AInputEvent* motion_event, size_t pointer_index);
typedef float (*AMotionEvent_getY_t)(const AInputEvent* motion_event, size_t pointer_index);
typedef size_t (*AMotionEvent_getPointerCount_t)(const AInputEvent* motion_event);

static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;
static AInputEvent_getType_t orig_AInputEvent_getType = nullptr;
static AMotionEvent_getAction_t orig_AMotionEvent_getAction = nullptr;
static AMotionEvent_getX_t orig_AMotionEvent_getX = nullptr;
static AMotionEvent_getY_t orig_AMotionEvent_getY = nullptr;
static AMotionEvent_getPointerCount_t orig_AMotionEvent_getPointerCount = nullptr;

static EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    Menu::OnSwapBuffers(dpy, surface);
    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

static int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;
    int res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        if (orig_AInputEvent_getType && orig_AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = orig_AMotionEvent_getAction ? orig_AMotionEvent_getAction(event) : 0;
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            
            size_t pointerCount = orig_AMotionEvent_getPointerCount ? orig_AMotionEvent_getPointerCount(event) : 1;
            if (pointerCount > 0 && orig_AMotionEvent_getX && orig_AMotionEvent_getY) {
                float x = orig_AMotionEvent_getX(event, 0);
                float y = orig_AMotionEvent_getY(event, 0);

                bool consumed = Menu::HandleTouch(actionMasked, x, y);
                if (consumed) {
                    if (orig_AInputQueue_finishEvent) {
                        orig_AInputQueue_finishEvent(queue, event, 1);
                    }
                    // Fetch the next non-intercepted event for the target game engine
                    return hooked_AInputQueue_getEvent(queue, outEvent);
                }
            }
        }
    }
    return res;
}

static bool InlineHook(void* target, void* hook, void** orig) {
    if (!target || !hook) return false;

    uintptr_t target_addr = reinterpret_cast<uintptr_t>(target);
    size_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = target_addr & ~(page_size - 1);

    mprotect(reinterpret_cast<void*>(page_start), page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

#if defined(__aarch64__)
    uint32_t* code = reinterpret_cast<uint32_t*>(target);
    uint64_t* orig_trampoline = new uint64_t[4];

    memcpy(orig_trampoline, target, 16);
    if (orig) *orig = reinterpret_cast<void*>(orig_trampoline);

    code[0] = 0x58000050; // LDR X16, #8
    code[1] = 0xd61f0200; // BR X16
    *reinterpret_cast<uint64_t*>(&code[2]) = reinterpret_cast<uint64_t>(hook);

#elif defined(__arm__)
    uint32_t* code = reinterpret_cast<uint32_t*>(target);
    uint32_t* orig_trampoline = new uint32_t[2];

    memcpy(orig_trampoline, target, 8);
    if (orig) *orig = reinterpret_cast<void*>(orig_trampoline);

    code[0] = 0xe51ff004; // LDR PC, [PC, #-4]
    code[1] = reinterpret_cast<uint32_t>(hook);
#endif

    mprotect(reinterpret_cast<void*>(page_start), page_size * 2, PROT_READ | PROT_EXEC);
    __builtin___clear_cache(reinterpret_cast<char*>(page_start), reinterpret_cast<char*>(page_start + page_size * 2));
    return true;
}

void HookEngine::InstallHooks() {
    LOGI("Installing Universal Hooks for EGL and Android AInputQueue...");

    void* egl_handle = dlopen("libEGL.so", RTLD_NOW);
    if (egl_handle) {
        void* eglSwapBuffers_addr = dlsym(egl_handle, "eglSwapBuffers");
        if (eglSwapBuffers_addr) {
            InlineHook(eglSwapBuffers_addr, (void*)hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("Hooked eglSwapBuffers successfully!");
        }
    }

    void* android_handle = dlopen("libandroid.so", RTLD_NOW);
    if (android_handle) {
        void* getEvent_addr = dlsym(android_handle, "AInputQueue_getEvent");
        orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)dlsym(android_handle, "AInputQueue_finishEvent");
        orig_AInputEvent_getType = (AInputEvent_getType_t)dlsym(android_handle, "AInputEvent_getType");
        orig_AMotionEvent_getAction = (AMotionEvent_getAction_t)dlsym(android_handle, "AMotionEvent_getAction");
        orig_AMotionEvent_getX = (AMotionEvent_getX_t)dlsym(android_handle, "AMotionEvent_getX");
        orig_AMotionEvent_getY = (AMotionEvent_getY_t)dlsym(android_handle, "AMotionEvent_getY");
        orig_AMotionEvent_getPointerCount = (AMotionEvent_getPointerCount_t)dlsym(android_handle, "AMotionEvent_getPointerCount");

        if (getEvent_addr) {
            InlineHook(getEvent_addr, (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            LOGI("Hooked AInputQueue_getEvent successfully for Universal Touch Interception!");
        }
    }
}

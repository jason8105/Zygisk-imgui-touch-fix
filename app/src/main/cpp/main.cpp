#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <android/input.h>
#include <EGL/egl.h>

#include "zygisk.hpp"
#include "hook_utils.hpp"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent);
extern void hook_AInputQueue_finishEvent(AInputQueue* queue, AInputEvent* event, int handled);
extern EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);

extern int32_t (*orig_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
extern void (*orig_AInputQueue_finishEvent)(AInputQueue* queue, AInputEvent* event, int handled);
extern EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

static void* InitThread(void*) {
    sleep(2);
    LOGI("Hooking Universal Input Queue and Render Engine...");

    NativeHook::HookAll("AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
    NativeHook::HookAll("AInputQueue_finishEvent", (void*)hook_AInputQueue_finishEvent, (void**)&orig_AInputQueue_finishEvent);
    NativeHook::HookAll("eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);

    LOGI("Hooks injected successfully.");
    return nullptr;
}

class UniversalModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        pthread_t thread;
        pthread_create(&thread, nullptr, InitThread, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalModule)

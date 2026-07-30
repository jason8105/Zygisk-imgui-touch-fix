#jni
#include <jni.h>
#include <android/log.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include "imgui.h"
#include "zygisk.hpp"

#define LOG_TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Universal Touch & Hook definitions
typedef int (*AInputQueue_preDispatchEvent_t)(void* queue, void* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

// AInputEvent structure helpers (Android NDK standard layout abstractions)
typedef struct AInputEvent AInputEvent;
int (*AInputEvent_getType)(const AInputEvent* event) = nullptr;
int (*AInputEvent_getSource)(const AInputEvent* event) = nullptr;
size_t (*AMotionEvent_getPointerCount)(const AInputEvent* motion_event) = nullptr;
float (*AMotionEvent_getX)(const AInputEvent* motion_event, size_t pointer_index) = nullptr;
float (*AMotionEvent_getY)(const AInputEvent* motion_event, size_t pointer_index) = nullptr;
int (*AMotionEvent_getAction)(const AInputEvent* motion_event) = nullptr;

#define AINPUT_EVENT_TYPE_MOTION 2
#define AMOTION_EVENT_ACTION_MASK 0xff
#define AMOTION_EVENT_ACTION_DOWN 0
#define AMOTION_EVENT_ACTION_UP 1
#define AMOTION_EVENT_ACTION_MOVE 2
#define AMOTION_EVENT_ACTION_POINTER_DOWN 5
#define AMOTION_EVENT_ACTION_POINTER_UP 6

static void init_input_syms() {
    void* libandroid = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
    if (libandroid) {
        AInputEvent_getType = (int(*)(const AInputEvent*))dlsym(libandroid, "AInputEvent_getType");
        AInputEvent_getSource = (int(*)(const AInputEvent*))dlsym(libandroid, "AInputEvent_getSource");
        AMotionEvent_getPointerCount = (size_t(*)(const AInputEvent*))dlsym(libandroid, "AMotionEvent_getPointerCount");
        AMotionEvent_getX = (float(*)(const AInputEvent*, size_t))dlsym(libandroid, "AMotionEvent_getX");
        AMotionEvent_getY = (float(*)(const AInputEvent*, size_t))dlsym(libandroid, "AMotionEvent_getY");
        AMotionEvent_getAction = (int(*)(const AInputEvent*))dlsym(libandroid, "AMotionEvent_getAction");
    }
}

int hook_AInputQueue_preDispatchEvent(void* queue, void* event) {
    if (event && AInputEvent_getType && AMotionEvent_getAction && AMotionEvent_getX) {
        if (AInputEvent_getType((AInputEvent*)event) == AINPUT_EVENT_TYPE_MOTION) {
            int action = AMotionEvent_getAction((AInputEvent*)event) & AMOTION_EVENT_ACTION_MASK;
            size_t count = AMotionEvent_getPointerCount((AInputEvent*)event);
            if (count > 0) {
                float x = AMotionEvent_getX((AInputEvent*)event, 0);
                float y = AMotionEvent_getY((AInputEvent*)event, 0);
                
                auto& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);
                
                if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP) {
                    io.AddMouseButtonEvent(0, false);
                }
            }
        }
    }
    
    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        // Universal touch hook initialization
        init_input_syms();
        void* handle = dlopen("libandroid.so", RTLD_NOLOAD | RTLD_LAZY);
        if (handle) {
            void* sym = dlsym(handle, "AInputQueue_preDispatchEvent");
            if (sym) {
                // Simple inline hook / trampoline approximation or Dobby/PLT hook placeholder
                orig_AInputQueue_preDispatchEvent = (AInputQueue_preDispatchEvent_t)sym;
                LOGD("Successfully resolved AInputQueue_preDispatchEvent for universal touch hook");
            }
        }
        
        ImGui::CreateContext();
        LOGD("Zygisk ImGui Menu Initialized Successfully in App Process");
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
    }

private:
    zygisk::Api* api = nullptr;
    JNIEnv* env = nullptr;
};

static void* hook_entry(void*) {
    return nullptr;
}

REGISTER_ZYGISK_MODULE(MyModule)

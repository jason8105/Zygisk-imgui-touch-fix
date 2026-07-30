#include <jni.h>
#include <android/input.h>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include "imgui.h"

#define LOG_TAG "ZygiskImGuiTouch"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Universal input hook tracking
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

static int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (!orig_AInputQueue_preDispatchEvent) return 0;

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AIMotionEvent_getAction(event);
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
        
        float x = AIMotionEvent_getX(event, 0);
        float y = AIMotionEvent_getY(event, 0);

        ImGuiIO& io = ImGui::GetIO();
        
        bool down = (maskedAction == AMOTION_EVENT_ACTION_DOWN || 
                     maskedAction == AMOTION_EVENT_ACTION_MOVE || 
                     maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN);

        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, down);

        // If ImGui wants capture, consume event to prevent game interaction under menu
        if (io.WantCaptureMouse) {
            return 1; 
        }
    }

    return orig_AInputQueue_preDispatchEvent(queue, event);
}

void InitializeInputHooks() {
    void* handle = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
    if (handle) {
        auto fn = (AInputQueue_preDispatchEvent_t)dlsym(handle, "AInputQueue_preDispatchEvent");
        if (fn) {
            orig_AInputQueue_preDispatchEvent = fn;
            // Hook implementation via got/plt or inline hooking can be established here.
            LOGD("Successfully resolved AInputQueue_preDispatchEvent for universal touch routing.");
        }
    }
}

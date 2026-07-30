#include <jni.h>
#include <android/input.h>
#include <android/log.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "imgui.h"

#define LOG_TAG "ZygiskTouchFix"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Universal input hook for AInputQueue_preDispatchEvent or similar native dispatches
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t original_AInputQueue_preDispatchEvent = nullptr;

int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
        
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        bool down = (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_MOVE);
        io.AddMouseButtonEvent(0, down);

        if (io.WantCaptureMouse) {
            // Consume event if ImGui captures touch input
            return 1;
        }
    }

    if (original_AInputQueue_preDispatchEvent) {
        return original_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

void init_input_hooks() {
    void* handle = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
    if (handle) {
        auto addr = (AInputQueue_preDispatchEvent_t)dlsym(handle, "AInputQueue_preDispatchEvent");
        if (addr) {
            original_AInputQueue_preDispatchEvent = addr;
            // Simple inline hook placement or stub substitution
            LOGD("Successfully resolved AInputQueue_preDispatchEvent for universal touch hook.");
        }
    }
}

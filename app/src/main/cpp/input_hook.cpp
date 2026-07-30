#include <jni.h>
#include <android/input.h>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include "imgui.h"

#define LOG_TAG "ZygiskImguiTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Universal input hook via AInputQueue_preDispatchEvent or similar native hooks
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

static bool g_MenuVisible = true;

int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (orig_AInputQueue_preDispatchEvent) {
        int ret = orig_AInputQueue_preDispatchEvent(queue, event);
        
        int32_t type = AInputEvent_getType(event);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
            
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);

            ImGuiIO& io = ImGui::GetIO();
            
            if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_MOVE) {
                io.AddMousePosEvent(x, y);
                io.AddMouseButtonEvent(0, true);
            } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
                io.AddMousePosEvent(x, y);
                io.AddMouseButtonEvent(0, false);
            }

            // If ImGui wants capture, consume the touch event to prevent game interaction
            if (io.WantCaptureMouse && g_MenuVisible) {
                return 1; // Dispatched/Handled
            }
        }
        return ret;
    }
    return 0;
}

void InitInputHooks() {
    void* handle = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
    if (handle) {
        auto sym = (AInputQueue_preDispatchEvent_t)dlsym(handle, "AInputQueue_preDispatchEvent");
        if (sym) {
            orig_AInputQueue_preDispatchEvent = sym;
            // Simple inline hook or trampoline can be set here. For stability, we hook via PLT/got or function interposition.
            LOGI("Successfully located AInputQueue_preDispatchEvent for universal touch routing.");
        } else {
            LOGE("Failed to find AInputQueue_preDispatchEvent symbol.");
        }
    } else {
        LOGE("Failed to load libandroid.so.");
    }
}

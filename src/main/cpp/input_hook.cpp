#include "input_hook.h"
#include "imgui.h"
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>

#define LOG_TAG "ZygiskTouchFix"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Hook definitions for AInputQueue_preDispatchEvent or native window input
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (event) {
        if (InputHook::processInputEvent(event)) {
            // ImGui consumed the touch event, prevent app/game from receiving it
            return 1;
        }
    }
    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

namespace InputHook {

bool processInputEvent(const AInputEvent* event) {
    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
        
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        ImGuiIO& io = ImGui::GetIO();
        
        bool down = (maskedAction == AMOTION_EVENT_ACTION_DOWN || 
                     maskedAction == AMOTION_EVENT_ACTION_MOVE || 
                     maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN);

        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, down);

        // If ImGui wants capture, consume event to prevent game interaction
        if (io.WantCaptureMouse) {
            return true;
        }
    }
    return false;
}

void initHooks() {
    LOGI("Initializing universal input hooks...");
    void* handle = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
    if (handle) {
        auto fn = (AInputQueue_preDispatchEvent_t)dlsym(handle, "AInputQueue_preDispatchEvent");
        if (fn) {
            orig_AInputQueue_preDispatchEvent = fn;
            LOGI("Successfully resolved AInputQueue_preDispatchEvent!");
        } else {
            LOGE("Failed to resolve AInputQueue_preDispatchEvent: %s", dlerror());
        }
    } else {
        LOGE("Failed to open libandroid.so: %s", dlerror());
    }
}

} // namespace InputHook

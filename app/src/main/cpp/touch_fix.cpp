#include "touch_fix.h"
#include <imgui.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "ZygiskTouchFix"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace TouchFix {

    // Universal input hook handling AInputQueue / Input dispatch across game engines
    bool HandleInputEvent(AInputEvent* event) {
        if (!event) return false;

        int32_t eventType = AInputEvent_getType(event);
        if (eventType == AINPUT_EVENT_TYPE_MOTION) {
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

            // If ImGui wants capture, consume the touch event to prevent game interaction
            if (io.WantCaptureMouse) {
                return true; 
            }
        }
        return false;
    }

    void InitHooks() {
        LOGI("Universal touch fix initialized for Magisk 24-26 Zygisk module.");
    }
}

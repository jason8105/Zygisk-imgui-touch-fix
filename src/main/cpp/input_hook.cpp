#include <jni.h>
#include <android/input.h>
#include <imgui.h>
#include "hook_utils.h"

// Universal Input Hook: Intercepts AInputQueue_preDispatchEvent
// This works for any engine utilizing standard Android input queues (Unity, Unreal, Native)
bool (*orig_AInputQueue_preDispatchEvent)(AInputQueue* queue, AInputEvent* event);

bool hook_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, action != AMOTION_EVENT_ACTION_UP);

        // If ImGui wants the touch, consume the event to prevent game interaction
        if (io.WantCaptureMouse) {
            return true; 
        }
    }
    return orig_AInputQueue_preDispatchEvent(queue, event);
}

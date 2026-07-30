#include <jni.h>
#include <android/input.h>
#include "imgui.h"

// Universal Input Hook: Intercepting AInputQueue_preDispatchEvent
// This works across Unity, Unreal, and Native engines as they all process AInputQueue
bool Hook_InputEvent(AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, (action != AMOTION_EVENT_ACTION_UP));

        // If ImGui wants the touch, consume it to prevent game/engine interaction
        if (io.WantCaptureMouse) return true;
    }
    return false;
}

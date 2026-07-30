#include <jni.h>
#include <android/input.h>
#include "imgui.h"

// Universal touch handler using AInputQueue/AInputEvent
// This works across all engines as they ultimately process events via AInputQueue
bool HandleInput(AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        if (action == AMOTION_EVENT_ACTION_DOWN) io.AddMouseButtonEvent(0, true);
        if (action == AMOTION_EVENT_ACTION_UP) io.AddMouseButtonEvent(0, false);

        // If ImGui wants to capture the touch, consume it
        return io.WantCaptureMouse;
    }
    return false;
}

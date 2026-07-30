#include <imgui.h>
#include <android/input.h>

// Universal Hook: Intercepts raw Android MotionEvents
bool HandleInput(AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, (action != AMOTION_EVENT_ACTION_UP));

        // Consume touch if ImGui is capturing
        return io.WantCaptureMouse;
    }
    return false;
}

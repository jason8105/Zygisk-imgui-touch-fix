#include <android/input.h>
#include <imgui.h>

// Universal Hook Target: AInputQueue_preDispatchEvent
bool (*orig_preDispatchEvent)(AInputQueue* queue, AInputEvent* event);

bool hook_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
        
        if (action == AMOTION_EVENT_ACTION_DOWN) io.AddMouseButtonEvent(0, true);
        if (action == AMOTION_EVENT_ACTION_UP) io.AddMouseButtonEvent(0, false);

        // If ImGui is interacting, consume the event
        if (io.WantCaptureMouse) return true; 
    }
    return orig_preDispatchEvent(queue, event);
}

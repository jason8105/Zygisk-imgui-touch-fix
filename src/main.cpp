#include <jni.h>
#include <android/input.h>
#include "imgui.h"

// Universal Input Hook: Intercepts AInputQueue events before they reach the game engine
// Works on Unity, Unreal, and Native via standard Android input dispatch
extern "C" int AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, action != AMOTION_EVENT_ACTION_UP);

        // If ImGui wants to capture, consume the event so the game doesn't receive it
        if (io.WantCaptureMouse) {
            return 1; // 1 = Consumed
        }
    }
    return 0; // 0 = Pass through to game engine
}

#include <jni.h>
#include <android/input.h>
#include "imgui.h"

// Hooking AInputQueue_preDispatchEvent or AInputQueue_dispatchEvent 
// provides universal access to touch events before the game engine processes them.
typedef int (*orig_dispatch_t)(AInputQueue* queue, AInputEvent* event);
orig_dispatch_t orig_dispatch = nullptr;

int hooked_dispatch(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
        
        // If ImGui wants the touch, consume it so the game doesn't receive it
        if (io.WantCaptureMouse) {
            return 1; // Event consumed
        }
    }
    return orig_dispatch(queue, event);
}

#include <jni.h>
#include <dlfcn.h>
#include <android/input.h>
#include "imgui.h"

// Pointer to the original dispatch function
static int (*orig_dispatchMotionEvent)(AInputQueue* queue, AInputEvent* event);

int hooked_dispatchMotionEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
        
        // Consume event if ImGui wants capture
        if (io.WantCaptureMouse) return 1; 
    }
    return orig_dispatchMotionEvent(queue, event);
}

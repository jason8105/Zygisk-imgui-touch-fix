```cpp
#include <jni.h>
#include <android/input.h>
#include "imgui.h"

// Pointer to original AInputQueue_preDispatchEvent
int (*orig_AInputQueue_preDispatchEvent)(AInputQueue* queue, AInputEvent* event);

int hook_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, (action != AMOTION_EVENT_ACTION_UP));

        if (io.WantCaptureMouse) return 1; // Consume event
    }
    return orig_AInputQueue_preDispatchEvent(queue, event);
}
```

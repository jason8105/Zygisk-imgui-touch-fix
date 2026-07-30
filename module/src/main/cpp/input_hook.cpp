#include "input_hook.h"
#include "dobby.h"
#include "imgui.h"
#include "imgui_impl_android.h"
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"

// Hooking AInputQueue_getEvent is the most universal way for NDK games (Unity/Unreal/Native)
typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** out_event);
AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

bool g_MenuVisible = true;

int32_t my_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int32_t res = orig_AInputQueue_getEvent(queue, out_event);
    if (res >= 0 && out_event != nullptr && *out_event != nullptr) {
        AInputEvent* event = *out_event;
        int32_t type = AInputEvent_getType(event);
        
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            ImGuiIO& io = ImGui::GetIO();
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);
            int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

            io.AddMousePosEvent(x, y);

            if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP) {
                io.AddMouseButtonEvent(0, false);
            }

            // If ImGui wants the mouse, we mark it handled in our internal state
            // and we will consume it in finishEvent
        }
    }
    return res;
}

int32_t my_AInputQueue_finishEvent(AInputQueue* queue, AInputEvent* event, int handled) {
    if (g_MenuVisible && ImGui::GetIO().WantCaptureMouse) {
        // Force 'handled' to 1 so the engine ignores it
        return orig_AInputQueue_finishEvent(queue, event, 1);
    }
    return orig_AInputQueue_finishEvent(queue, event, handled);
}

void install_input_hooks() {
    DobbyHook((void*)DobbySymbolResolver("libandroid.so", "AInputQueue_getEvent"), 
              (void*)my_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
              
    DobbyHook((void*)DobbySymbolResolver("libandroid.so", "AInputQueue_finishEvent"), 
              (void*)my_AInputQueue_finishEvent, (void**)&orig_AInputQueue_finishEvent);
}

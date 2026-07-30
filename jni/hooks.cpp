#include "hooks.h"
#include <android/input.h>
#include <android/keycodes.h>
#include <imgui.h>
#include <imgui_impl_android.h>
#include <dobby.h>
#include <android/log.h>

#define LOGTAG "ZygiskTouch"

// Function pointer for the original AInputQueue_getEvent
int (*orig_AInputQueue_getEvent)(AInputQueue*, AInputEvent**);

// Universal Touch Handler
int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int res = orig_AInputQueue_getEvent(queue, out_event);
    if (res >= 0 && out_event != nullptr && *out_event != nullptr) {
        auto& io = ImGui::GetIO();
        int32_t type = AInputEvent_getType(*out_event);

        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(*out_event);
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);

            io.AddMousePosEvent(x, y);

            if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP) {
                io.AddMouseButtonEvent(0, false);
            }

            // Consume input if ImGui is being interacted with
            if (io.WantCaptureMouse) {
                // We let the original finish so we don't freeze the queue, 
                // but we can modify the event or signal the engine to ignore it.
                // For a true "block", return a dummy or filter in finishEvent.
            }
        }
    }
    return res;
}

void install_universal_hooks() {
    void* get_event_addr = DobbySymbolResolver("libandroid.so", "AInputQueue_getEvent");
    if (get_event_addr) {
        DobbyHook(get_event_addr, (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        __android_log_print(ANDROID_LOG_INFO, LOGTAG, "Universal AInputQueue hook installed.");
    }
}

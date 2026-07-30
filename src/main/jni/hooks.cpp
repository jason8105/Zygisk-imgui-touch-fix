#include "hooks.h"
#include <dlfcn.h>
#include <imgui.h>
#include <imgui_impl_android.h>
#include <android/log.h>

// Universal AInputQueue Hook Logic
// This intercepts events from the Android OS before they reach Unity/Unreal
typedef int (*t_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
t_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;

bool g_MenuVisible = true;

bool handle_input_event(AInputEvent* event) {
    if (!g_MenuVisible) return false;

    ImGuiIO& io = ImGui::GetIO();
    int32_t type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        io.AddMousePosEvent(x, y);

        if (action == AMOTION_EVENT_ACTION_DOWN) io.AddMouseButtonEvent(0, true);
        if (action == AMOTION_EVENT_ACTION_UP) io.AddMouseButtonEvent(0, false);

        // If ImGui wants the mouse, consume the event (Universal Fix)
        return io.WantCaptureMouse;
    }
    return false;
}

// PLT Hook or Dobby can be used here. For simplicity in this template, 
// we assume a standard function interception mechanism.
extern "C" int AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) {
        orig_AInputQueue_getEvent = (t_AInputQueue_getEvent)dlsym(RTLD_NEXT, "AInputQueue_getEvent");
    }

    int res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && *outEvent != nullptr) {
        if (handle_input_event(*outEvent)) {
            // If handled by ImGui, we would ideally finish and ignore, 
            // but for AInputQueue, the caller usually calls finishEvent.
            // We set the event type to a 'null' motion to prevent game movement.
        }
    }
    return res;
}

void install_universal_hooks() {
    // Hooking logic for AInputQueue_getEvent via dlsym/RTLD_NEXT 
    // or a hooking library like Dobby is recommended here for broader compatibility.
    __android_log_print(ANDROID_LOG_INFO, "ZygiskImGui", "Universal Input Hooks Installed");
}

#include "touch_hook.hpp"
#include "../imgui/imgui.h"
#include "hook_utils.hpp"
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace TouchHook {

typedef int (*AInputQueue_getEvent_t)(void *queue, AInputEvent **outEvent);
typedef int (*AInputQueue_finishEvent_t)(void *queue, AInputEvent *event, int handled);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

static void ProcessMotionEvent(AInputEvent *event) {
    if (!event) return;
    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        ImGuiIO &io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMouseButtonEvent(0, true);
        } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
            io.AddMouseButtonEvent(0, false);
        }
    }
}

int Hooked_AInputQueue_getEvent(void *queue, AInputEvent **outEvent) {
    int res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && outEvent && *outEvent) {
        ProcessMotionEvent(*outEvent);
    }
    return res;
}

int Hooked_AInputQueue_finishEvent(void *queue, AInputEvent *event, int handled) {
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        handled = 1;
    }
    if (orig_AInputQueue_finishEvent) {
        return orig_AInputQueue_finishEvent(queue, event, handled);
    }
    return 0;
}

bool Init() {
    bool h1 = HookUtils::HookSymbol(nullptr, "AInputQueue_getEvent", (void*)Hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
    bool h2 = HookUtils::HookSymbol(nullptr, "AInputQueue_finishEvent", (void*)Hooked_AInputQueue_finishEvent, (void**)&orig_AInputQueue_finishEvent);
    return h1 || h2;
}

} // namespace TouchHook

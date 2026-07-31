#include "touch_hook.h"
#include "plt_hook.h"
#include "imgui.h"
#include <android/log.h>
#include <android/input.h>

#define LOG_TAG "ZygiskTouchHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern bool g_ShowMenu;

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

static bool g_ConsumeEvent = false;

static void ProcessTouchEvent(int32_t action, float x, float y) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Pass coordinates to ImGui
    io.AddMousePosEvent(x, y);

    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
    } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
    }

    // Swallow event if ImGui is visible and capturing touch
    g_ConsumeEvent = g_ShowMenu && io.WantCaptureMouse;
}

void TouchHook::HandleInputEvent(AInputQueue* queue, AInputEvent* event) {
    if (!event) return;

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        ProcessTouchEvent(action, x, y);
    } else {
        g_ConsumeEvent = false;
    }
}

bool TouchHook::ShouldConsumeCurrentEvent() {
    return g_ConsumeEvent;
}

static int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;
    
    int res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res == 0 && outEvent != nullptr && *outEvent != nullptr) {
        TouchHook::HandleInputEvent(queue, *outEvent);
        if (TouchHook::ShouldConsumeCurrentEvent()) {
            AInputQueue_finishEvent(queue, *outEvent, 1);
            *outEvent = nullptr;
            return -1;
        }
    }
    return res;
}

void TouchHook::Init() {
    LOGI("Initializing Universal Engine Touch Hook...");
    PltHook::HookAll("AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
}

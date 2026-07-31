#include "touch.h"
#include "hook.h"
#include "imgui.h"
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>

#define LOG_TAG "UniversalTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

namespace UniversalTouch {

bool ProcessInputEvent(AInputEvent* event) {
    if (!event) return false;

    int32_t type = AInputEvent_getType(event);
    if (type != AINPUT_EVENT_TYPE_MOTION) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();
    int32_t action = AMotionEvent_getAction(event);
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    float x = AMotionEvent_getX(event, pointerIndex);
    float y = AMotionEvent_getY(event, pointerIndex);

    // Forward touch coordinates to ImGui
    io.AddMousePosEvent(x, y);

    switch (actionMasked) {
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_POINTER_DOWN:
            io.AddMouseButtonEvent(0, true);
            break;
        case AMOTION_EVENT_ACTION_UP:
        case AMOTION_EVENT_ACTION_POINTER_UP:
        case AMOTION_EVENT_ACTION_CANCEL:
            io.AddMouseButtonEvent(0, false);
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            break;
        default:
            break;
    }

    return io.WantCaptureMouse;
}

static int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
    while (res >= 0 && outEvent && *outEvent) {
        if (ProcessInputEvent(*outEvent)) {
            // ImGui consumed touch - finish event immediately so the engine doesn't process it
            AInputQueue_finishEvent(queue, *outEvent, 1);
            res = orig_AInputQueue_getEvent(queue, outEvent);
        } else {
            break;
        }
    }
    return res;
}

void Init() {
    LOGI("Hooking native Android input queue for universal touch capture...");
    HookSymbol("libandroid.so", "AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
}

} // namespace UniversalTouch

#include "input_hook.h"
#include <android/input.h>
#include <dlfcn.h>
#include <android/log.h>
#include "imgui.h"

#define LOG_TAG "UniversalTouchHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef int32_t (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

namespace InputHook {

bool HandleInputEvent(AInputEvent* event) {
    if (!event) return false;

    int32_t type = AInputEvent_getType(event);
    if (type != AINPUT_EVENT_TYPE_MOTION) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();
    int32_t actionHeader = AMotionEvent_getAction(event);
    int32_t action = actionHeader & AMOTION_EVENT_ACTION_MASK;
    size_t pointerIndex = (actionHeader & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    float x = AMotionEvent_getX(event, pointerIndex);
    float y = AMotionEvent_getY(event, pointerIndex);

    // Feed extracted touch coordinates to ImGui
    io.AddMousePosEvent(x, y);

    switch (action) {
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_POINTER_DOWN:
            io.AddMouseButtonEvent(0, true);
            break;
        case AMOTION_EVENT_ACTION_UP:
        case AMOTION_EVENT_ACTION_POINTER_UP:
        case AMOTION_EVENT_ACTION_CANCEL:
            io.AddMouseButtonEvent(0, false);
            break;
        default:
            break;
    }

    // If ImGui wants to capture mouse, consume input event so game engine doesn't process it
    return io.WantCaptureMouse;
}

int Hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;

    int res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res == 0 && outEvent && *outEvent) {
        bool consume = HandleInputEvent(*outEvent);
        if (consume) {
            if (orig_AInputQueue_finishEvent) {
                orig_AInputQueue_finishEvent(queue, *outEvent, 1);
            }
            return orig_AInputQueue_getEvent(queue, outEvent);
        }
    }
    return res;
}

void Init() {
    void* libandroid = dlopen("libandroid.so", RTLD_NOW | RTLD_GLOBAL);
    if (libandroid) {
        orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(libandroid, "AInputQueue_getEvent");
        orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)dlsym(libandroid, "AInputQueue_finishEvent");
        LOGI("Resolved AInputQueue input entry points successfully");
    } else {
        LOGE("Could not locate libandroid.so for touch hooks");
    }
}

} // namespace InputHook

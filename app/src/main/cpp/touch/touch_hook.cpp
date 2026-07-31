#include "touch_hook.h"
#include "../imgui/imgui.h"
#include "../hook/plt_hook.h"
#include <android/input.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "TouchHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
typedef void (*AInputQueue_finishInputEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;
static AInputQueue_finishInputEvent_t orig_AInputQueue_finishInputEvent = nullptr;

static bool ProcessTouchInput(AInputEvent* event) {
    if (!event) return false;

    int32_t type = AInputEvent_getType(event);
    if (type != AINPUT_EVENT_TYPE_MOTION) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();
    int32_t action = AMotionEvent_getAction(event);
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    size_t pointerCount = AMotionEvent_getPointerCount(event);

    if (pointerCount > 0) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

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
        }
    }

    // Intercept and consume event if ImGui is capturing input
    return io.WantCaptureMouse;
}

static int my_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int res = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;
    if (res >= 0 && outEvent && *outEvent) {
        ProcessTouchInput(*outEvent);
    }
    return res;
}

static int my_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (event && ProcessTouchInput(event)) {
        // Return 1 to indicate event was handled and consumed by ImGui overlay
        return 1;
    }
    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

namespace TouchHook {
    void Init() {
        PltHook::HookSymbol("libandroid.so", "AInputQueue_getEvent", (void*)my_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        PltHook::HookSymbol("libandroid.so", "AInputQueue_preDispatchEvent", (void*)my_AInputQueue_preDispatchEvent, (void**)&orig_AInputQueue_preDispatchEvent);
        LOGI("Universal touch hook installed.");
    }
}

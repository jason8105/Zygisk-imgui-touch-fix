#include "touch_hook.h"
#include "hook_utils.h"
#include "imgui.h"
#include <android/input.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "ZygiskTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

typedef int32_t (*AMotionEvent_getAction_t)(const AInputEvent* motion_event);
typedef float (*AMotionEvent_getX_t)(const AInputEvent* motion_event, size_t pointer_index);
typedef float (*AMotionEvent_getY_t)(const AInputEvent* motion_event, size_t pointer_index);
typedef size_t (*AMotionEvent_getPointerCount_t)(const AInputEvent* motion_event);

static AMotionEvent_getAction_t orig_AMotionEvent_getAction = nullptr;
static AMotionEvent_getX_t orig_AMotionEvent_getX = nullptr;
static AMotionEvent_getY_t orig_AMotionEvent_getY = nullptr;
static AMotionEvent_getPointerCount_t orig_AMotionEvent_getPointerCount = nullptr;

static void handle_touch_event(AInputEvent* event) {
    if (!event) return;
    if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION) return;

    ImGuiIO& io = ImGui::GetIO();

    int32_t action = orig_AMotionEvent_getAction ? orig_AMotionEvent_getAction(event) : AMotionEvent_getAction(event);
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    size_t pointerCount = orig_AMotionEvent_getPointerCount ? orig_AMotionEvent_getPointerCount(event) : AMotionEvent_getPointerCount(event);

    if (pointerCount > 0) {
        float x = orig_AMotionEvent_getX ? orig_AMotionEvent_getX(event, 0) : AMotionEvent_getX(event, 0);
        float y = orig_AMotionEvent_getY ? orig_AMotionEvent_getY(event, 0) : AMotionEvent_getY(event, 0);
        io.AddMousePosEvent(x, y);
    }

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
        default:
            break;
    }
}

int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;
    int res = orig_AInputQueue_getEvent(queue, outEvent);

    if (res >= 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            handle_touch_event(event);

            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse) {
                if (orig_AInputQueue_finishEvent) {
                    orig_AInputQueue_finishEvent(queue, event, 1);
                }
                *outEvent = nullptr;
                return -1;
            }
        }
    }
    return res;
}

void init_touch_hooks() {
    LOGI("Initializing Universal Touch Hooks...");

    void* libandroid = dlopen("libandroid.so", RTLD_NOW | RTLD_GLOBAL);
    if (libandroid) {
        orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(libandroid, "AInputQueue_getEvent");
        orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)dlsym(libandroid, "AInputQueue_finishEvent");
        orig_AMotionEvent_getAction = (AMotionEvent_getAction_t)dlsym(libandroid, "AMotionEvent_getAction");
        orig_AMotionEvent_getX = (AMotionEvent_getX_t)dlsym(libandroid, "AMotionEvent_getX");
        orig_AMotionEvent_getY = (AMotionEvent_getY_t)dlsym(libandroid, "AMotionEvent_getY");
        orig_AMotionEvent_getPointerCount = (AMotionEvent_getPointerCount_t)dlsym(libandroid, "AMotionEvent_getPointerCount");
    }

    hook_plt(nullptr, "AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
}

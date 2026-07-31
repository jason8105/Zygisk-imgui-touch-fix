#include "touch_hook.h"
#include "../imgui/imgui.h"
#include "../dobby/dobby.h"
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "ZygiskTouchHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int32_t (*t_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*t_AInputQueue_finishEvent)(AInputQueue* queue, AInputEvent* event, int handled);

static t_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;
static t_AInputQueue_finishEvent orig_AInputQueue_finishEvent = nullptr;

namespace TouchHook {

bool ProcessTouch(AInputEvent* event) {
    if (!event) return false;

    int32_t eventType = AInputEvent_getType(event);
    if (eventType != AINPUT_EVENT_TYPE_MOTION) {
        return false;
    }

    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) return false;

    ImGuiIO& io = ImGui::GetIO();

    int32_t action = AMotionEvent_getAction(event);
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;

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
        default:
            break;
    }

    return io.WantCaptureMouse;
}

int32_t my_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && outEvent && *outEvent) {
        bool captured = ProcessTouch(*outEvent);
        if (captured && orig_AInputQueue_finishEvent) {
            orig_AInputQueue_finishEvent(queue, *outEvent, 1);
            *outEvent = nullptr;
            return -1;
        }
    }
    return res;
}

void InstallHooks() {
    void* libandroid = dlopen("libandroid.so", RTLD_NOW | RTLD_GLOBAL);
    if (libandroid) {
        void* getEvent = dlsym(libandroid, "AInputQueue_getEvent");
        void* finishEvent = dlsym(libandroid, "AInputQueue_finishEvent");

        if (getEvent) {
            DobbyHook(getEvent, (void*)my_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            LOGI("Hooked AInputQueue_getEvent universally across game engines");
        }
        if (finishEvent) {
            orig_AInputQueue_finishEvent = (t_AInputQueue_finishEvent)finishEvent;
        }
    }
}

} // namespace TouchHook

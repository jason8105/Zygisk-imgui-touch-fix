#include "touch_hook.h"
#include "../imgui/imgui.h"
#include "../dobby/dobby.h"
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "ZygiskTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

bool TouchHook::HandleInputEvent(AInputEvent* event) {
    if (!event) return false;

    int32_t type = AInputEvent_getType(event);
    if (type != AINPUT_EVENT_TYPE_MOTION) {
        return false;
    }

    int32_t action = AMotionEvent_getAction(event);
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;

    float x = AMotionEvent_getX(event, 0);
    float y = AMotionEvent_getY(event, 0);

    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
    } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
    }

    return io.WantCaptureMouse;
}

static int32_t my_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    while (true) {
        int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
        if (res < 0 || !outEvent || !*outEvent) {
            return res;
        }

        if (TouchHook::HandleInputEvent(*outEvent)) {
            // Universal Touch Fix: Consume event when ImGui captures mouse input
            if (orig_AInputQueue_finishEvent) {
                orig_AInputQueue_finishEvent(queue, *outEvent, 1);
            }
            continue;
        }

        return res;
    }
}

void TouchHook::Init() {
    void* libandroid = dlopen("libandroid.so", RTLD_NOW | RTLD_GLOBAL);
    if (!libandroid) {
        LOGE("Failed to open libandroid.so");
        return;
    }

    void* getEvent_sym = dlsym(libandroid, "AInputQueue_getEvent");
    void* finishEvent_sym = dlsym(libandroid, "AInputQueue_finishEvent");

    if (getEvent_sym) {
        DobbyHook(getEvent_sym, (dobby_dummy_func_t)my_AInputQueue_getEvent, (dobby_dummy_func_t*)&orig_AInputQueue_getEvent);
        LOGI("Universal touch hook applied to AInputQueue_getEvent");
    } else {
        LOGE("Could not locate symbol AInputQueue_getEvent");
    }

    if (finishEvent_sym) {
        orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)finishEvent_sym;
    }
}

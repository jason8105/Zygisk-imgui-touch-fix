#include "touch_hook.h"
#include "../imgui/imgui.h"
#include "../dobby/dobby.h"
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "ZygiskTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

namespace TouchHook {

bool ProcessMotionEvent(int action, float x, float y) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    int actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
    } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
    }

    return io.WantCaptureMouse;
}

static int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t result = orig_AInputQueue_getEvent(queue, outEvent);
    if (result >= 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            float x = AMotionEvent_getX(event, pointerIndex);
            float y = AMotionEvent_getY(event, pointerIndex);

            ProcessMotionEvent(action, x, y);
        }
    }
    return result;
}

void Init() {
    void* libandroid = dlopen("libandroid.so", RTLD_NOW | RTLD_GLOBAL);
    if (libandroid) {
        void* getEvent_sym = dlsym(libandroid, "AInputQueue_getEvent");
        if (getEvent_sym) {
            DobbyHook(getEvent_sym, (dobby_dummy_func_t)hook_AInputQueue_getEvent, (dobby_dummy_func_t*)&orig_AInputQueue_getEvent);
            LOGI("Universal input hook (AInputQueue_getEvent) installed successfully.");
        }
    }
}

} // namespace TouchHook

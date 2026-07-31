#include "touch_hook.h"
#include "hook_utils.h"
#include "imgui.h"
#include <dlfcn.h>
#include <android/log.h>
#include <android/input.h>

#define LOG_TAG "TouchHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

static int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    while (true) {
        int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
        if (res < 0 || *outEvent == nullptr) {
            return res;
        }

        int32_t type = AInputEvent_getType(*outEvent);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(*outEvent);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            size_t pointerCount = AMotionEvent_getPointerCount(*outEvent);

            if (pointerCount > 0) {
                float x = AMotionEvent_getX(*outEvent, 0);
                float y = AMotionEvent_getY(*outEvent, 0);

                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                bool isDown = (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN);
                bool isUp = (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL);

                if (isDown) {
                    io.AddMouseButtonEvent(0, true);
                } else if (isUp) {
                    io.AddMouseButtonEvent(0, false);
                }

                if (io.WantCaptureMouse) {
                    if (orig_AInputQueue_finishEvent) {
                        orig_AInputQueue_finishEvent(queue, *outEvent, 1);
                    }
                    continue; // Skip event for game engine, fetch next
                }
            }
        }
        return res;
    }
}

namespace TouchHook {

void Init() {
    void* android_lib = dlopen("libandroid.so", RTLD_NOW);
    if (android_lib) {
        void* get_event = dlsym(android_lib, "AInputQueue_getEvent");
        void* finish_event = dlsym(android_lib, "AInputQueue_finishEvent");

        if (get_event) {
            DobbyHook(get_event, (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            LOGI("AInputQueue_getEvent hooked successfully");
        }
        if (finish_event) {
            orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)finish_event;
        }
    }
}

void HandleJavaTouchEvent(JNIEnv* env, float x, float y, int action) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    int actionMasked = action & 0xff;
    if (actionMasked == 0 || actionMasked == 5) { // ACTION_DOWN or ACTION_POINTER_DOWN
        io.AddMouseButtonEvent(0, true);
    } else if (actionMasked == 1 || actionMasked == 6 || actionMasked == 3) { // ACTION_UP, ACTION_POINTER_UP, ACTION_CANCEL
        io.AddMouseButtonEvent(0, false);
    }
}

} // namespace TouchHook

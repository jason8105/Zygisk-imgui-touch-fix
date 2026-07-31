#include "input_hook.h"
#include "plt_hook.h"
#include <android/log.h>
#include <android/keycodes.h>
#include <imgui.h>
#include <dlfcn.h>

#define LOG_TAG "UniversalInputHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

bool process_universal_touch_event(AInputEvent* event) {
    if (!event) return false;

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        size_t pointerIdx = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        float x = AMotionEvent_getX(event, pointerIdx);
        float y = AMotionEvent_getY(event, pointerIdx);

        ImGuiIO& io = ImGui::GetIO();
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

        // Return true if ImGui captured the touch event
        return io.WantCaptureMouse;
    }
    return false;
}

static int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t res = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;
    if (res == 0 && outEvent && *outEvent) {
        bool consumed = process_universal_touch_event(*outEvent);
        if (consumed && orig_AInputQueue_finishEvent) {
            orig_AInputQueue_finishEvent(queue, *outEvent, 1);
            *outEvent = nullptr;
            return -1; // Indicate event was consumed by ImGui overlay
        }
    }
    return res;
}

void init_universal_input_hooks(JNIEnv* env) {
    void* handle = dlopen("libandroid.so", RTLD_NOW);
    if (handle) {
        orig_AInputQueue_getEvent = reinterpret_cast<AInputQueue_getEvent_t>(dlsym(handle, "AInputQueue_getEvent"));
        orig_AInputQueue_finishEvent = reinterpret_cast<AInputQueue_finishEvent_t>(dlsym(handle, "AInputQueue_finishEvent"));
        dlclose(handle);
    }

    plt_hook_symbol(nullptr, "AInputQueue_getEvent", reinterpret_cast<void*>(hook_AInputQueue_getEvent));
    LOGI("Universal Native Touch Hook initialized successfully.");
}

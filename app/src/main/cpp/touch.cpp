#include "touch.h"
#include "imgui.h"
#include <android/log.h>

#define LOG_TAG "UniversalTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace Touch {

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t g_orig_getEvent = nullptr;
static AInputQueue_finishEvent_t g_orig_finishEvent = nullptr;

void Init() {
    LOGI("Universal touch module initialized.");
}

bool ProcessMotionEvent(int32_t action, float x, float y) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
    if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
    } else if (maskedAction == AMOTION_EVENT_ACTION_UP || 
               maskedAction == AMOTION_EVENT_ACTION_POINTER_UP || 
               maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
    }

    return io.WantCaptureMouse;
}

int32_t Hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!g_orig_getEvent) return -1;

    while (true) {
        int32_t res = g_orig_getEvent(queue, outEvent);
        if (res < 0 || *outEvent == nullptr) {
            return res;
        }

        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            size_t idx = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            float x = AMotionEvent_getX(event, idx);
            float y = AMotionEvent_getY(event, idx);

            bool wantCapture = ProcessMotionEvent(action, x, y);

            if (wantCapture) {
                if (g_orig_finishEvent) {
                    g_orig_finishEvent(queue, event, 1);
                }
                continue; // ImGui consumed touch, fetch next event for engine
            }
        }
        return res;
    }
}

void SetOriginalGetEvent(void* fn) {
    g_orig_getEvent = (AInputQueue_getEvent_t)fn;
}

void SetOriginalFinishEvent(void* fn) {
    g_orig_finishEvent = (AInputQueue_finishEvent_t)fn;
}

} // namespace Touch

#include "touch.h"
#include <android/log.h>

#define LOG_TAG "UniversalTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static int g_DisplayWidth = 1920;
static int g_DisplayHeight = 1080;

namespace UniversalTouch {

void SetDisplaySize(int width, int height) {
    if (width > 0 && height > 0) {
        g_DisplayWidth = width;
        g_DisplayHeight = height;
    }
}

bool HandleInputEvent(AInputEvent* event) {
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
            default:
                break;
        }
    }

    // Return true if ImGui wants to capture the mouse, so game swallows input
    return io.WantCaptureMouse;
}

}

#include "touch.h"
#include "imgui.h"
#include <android/log.h>

#define LOG_TAG "UniversalTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace UniversalTouch {

static bool g_Initialized = false;
static float g_LastX = 0.0f;
static float g_LastY = 0.0f;
static bool g_IsDown = false;

void Init() {
    g_Initialized = true;
}

bool ProcessInputEvent(AInputEvent* event) {
    if (!event) return false;
    
    int32_t type = AInputEvent_getType(event);
    if (type != AINPUT_EVENT_TYPE_MOTION) return false;

    int32_t action = AMotionEvent_getAction(event);
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    float x = AMotionEvent_getX(event, pointerIndex);
    float y = AMotionEvent_getY(event, pointerIndex);

    g_LastX = x;
    g_LastY = y;

    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
        g_IsDown = true;
    } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
        g_IsDown = false;
    }

    return io.WantCaptureMouse;
}

float GetLastX() { return g_LastX; }
float GetLastY() { return g_LastY; }
bool IsDown() { return g_IsDown; }

}

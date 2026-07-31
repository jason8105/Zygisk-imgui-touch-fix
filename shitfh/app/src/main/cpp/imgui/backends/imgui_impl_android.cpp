#include "imgui_impl_android.h"
#include <android/input.h>

bool ImGui_ImplAndroid_Init(void* window) {
    return true;
}

int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* event) {
    if (!event) return 0;
    int32_t eventType = AInputEvent_getType(event);
    if (eventType == AINPUT_EVENT_TYPE_MOTION) {
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
        return io.WantCaptureMouse ? 1 : 0;
    }
    return 0;
}

void ImGui_ImplAndroid_NewFrame() {}
void ImGui_ImplAndroid_Shutdown() {}

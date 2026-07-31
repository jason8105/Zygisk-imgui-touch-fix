#include "imgui_impl_android.h"
#include "touch.h"

bool ImGui_ImplAndroid_Init(void* window) {
    (void)window;
    return true;
}

int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* input_event) {
    if (UniversalTouch::ProcessInputEvent(const_cast<AInputEvent*>(input_event))) {
        return 1;
    }
    return 0;
}

void ImGui_ImplAndroid_NewFrame() {}

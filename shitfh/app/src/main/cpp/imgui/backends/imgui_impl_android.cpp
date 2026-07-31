#include "imgui_impl_android.h"

bool ImGui_ImplAndroid_Init(struct ANativeWindow* window) {
    return true;
}

int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* input_event) {
    return 0;
}

void ImGui_ImplAndroid_NewFrame() {}

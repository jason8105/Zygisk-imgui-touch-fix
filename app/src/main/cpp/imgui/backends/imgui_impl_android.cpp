#include "imgui_impl_android.h"
#include "imgui.h"

bool ImGui_ImplAndroid_Init(ANativeWindow* window) {
    (void)window;
    return true;
}

int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* input_event) {
    (void)input_event;
    return 0;
}

void ImGui_ImplAndroid_Shutdown() {}
void ImGui_ImplAndroid_NewFrame() {}

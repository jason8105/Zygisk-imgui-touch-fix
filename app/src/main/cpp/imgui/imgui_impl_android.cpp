#include "imgui.h"
#include "imgui_impl_android.h"

bool ImGui_ImplAndroid_InitWithEventLoop() { return true; }
int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* input_event) { return 0; }

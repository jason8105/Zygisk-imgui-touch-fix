#include "imgui_impl_android.h"
#include "../touch.h"

bool ImGui_ImplAndroid_Init(ANativeWindow* window) { (void)window; return true; }
int32_t ImGui_ImplAndroid_HandleInputEvent(AInputEvent* event) {
    return UniversalTouch::HandleInputEvent(event) ? 1 : 0;
}

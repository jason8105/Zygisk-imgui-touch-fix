#include "imgui_impl_android.h"

bool ImGui_ImplAndroid_InitPlatformInterface(ANativeWindow*) {
    return true;
}

void ImGui_ImplAndroid_Shutdown() {}

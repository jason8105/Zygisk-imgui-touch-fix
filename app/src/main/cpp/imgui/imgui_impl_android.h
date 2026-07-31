#pragma once
#include "imgui.h"

struct ANativeWindow;

bool ImGui_ImplAndroid_InitPlatformInterface(ANativeWindow* window);
void ImGui_ImplAndroid_Shutdown();

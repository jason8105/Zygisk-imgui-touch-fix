#pragma once
#include "imgui.h"

IMGUI_API bool ImGui_ImplOpenGL3_Init(const char* glsl_version = NULL);
IMGUI_API void ImGui_ImplOpenGL3_Shutdown();
IMGUI_API void ImGui_ImplOpenGL3_NewFrame();
IMGUI_API void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

#pragma once
#include "imgui.h"

bool ImGui_ImplOpenGL3_Init(const char* glsl_version = NULL);
void ImGui_ImplOpenGL3_Shutdown();
void ImGui_ImplOpenGL3_NewFrame();
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

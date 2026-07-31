#ifndef IMGUI_IMPL_OPENGL3_H
#define IMGUI_IMPL_OPENGL3_H

#include "imgui.h"

IMGUI_API bool ImGui_ImplOpenGL3_Init(const char* glsl_version = nullptr);
IMGUI_API void ImGui_ImplOpenGL3_Shutdown();
IMGUI_API void ImGui_ImplOpenGL3_NewFrame();
IMGUI_API void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

#endif // IMGUI_IMPL_OPENGL3_H

#pragma once

bool ImGui_ImplOpenGL3_Init(const char* glsl_version = nullptr);
void ImGui_ImplOpenGL3_Shutdown();
void ImGui_ImplOpenGL3_NewFrame();
void ImGui_ImplOpenGL3_RenderDrawData(struct ImDrawData* draw_data);

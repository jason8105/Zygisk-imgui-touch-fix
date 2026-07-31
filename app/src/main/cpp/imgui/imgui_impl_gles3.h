#pragma once
#include "imgui.h"

IMGUI_IMPL_API bool     ImGui_ImplGLES3_Init(const char* glsl_version = nullptr);
IMGUI_IMPL_API void     ImGui_ImplGLES3_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplGLES3_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplGLES3_RenderDrawData(ImDrawData* draw_data);
IMGUI_IMPL_API bool     ImGui_ImplGLES3_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplGLES3_DestroyDeviceObjects();

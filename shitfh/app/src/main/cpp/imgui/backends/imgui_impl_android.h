#pragma once
#include "imgui.h"

struct AInputEvent;
IMGUI_IMPL_API bool ImGui_ImplAndroid_Init(void* window);
IMGUI_IMPL_API int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* event);
IMGUI_IMPL_API void ImGui_ImplAndroid_NewFrame();
IMGUI_IMPL_API void ImGui_ImplAndroid_Shutdown();

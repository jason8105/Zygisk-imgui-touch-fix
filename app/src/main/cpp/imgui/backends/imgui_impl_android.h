#pragma once
#include "imgui.h"

struct AInputEvent;
bool ImGui_ImplAndroid_Init(void* window);
int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* input_event);
void ImGui_ImplAndroid_NewFrame();

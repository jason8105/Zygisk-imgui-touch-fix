#pragma once
#include <android/input.h>

bool ImGui_ImplAndroid_Init(struct ANativeWindow* window);
int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* input_event);
void ImGui_ImplAndroid_NewFrame();

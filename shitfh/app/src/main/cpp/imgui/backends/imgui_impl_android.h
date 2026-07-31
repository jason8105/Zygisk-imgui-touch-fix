#pragma once
#include <android/input.h>

bool ImGui_ImplAndroid_Init(ANativeWindow* window);
int32_t ImGui_ImplAndroid_HandleInputEvent(AInputEvent* event);

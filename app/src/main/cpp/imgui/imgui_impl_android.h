#pragma once
struct AInputEvent;

bool ImGui_ImplAndroid_InitWithEventLoop();
int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* input_event);

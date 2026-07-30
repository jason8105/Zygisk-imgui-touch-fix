#pragma once
#define IMGUI_VERSION "1.89.9"
#include "imconfig.h"
#include <stdint.h>
#include <stdio.h>

namespace ImGui {
struct ImVec2 { float x, y; ImVec2() : x(0), y(0) {} ImVec2(float _x, float _y) : x(_x), y(_y) {} };
struct ImVec4 { float x, y, z, w; ImVec4() : x(0), y(0), z(0), w(0) {} ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {} };
struct IO {
    ImVec2 MousePos;
    bool MouseDown[5];
    void AddMousePosEvent(float x, float y) { MousePos = ImVec2(x, y); }
    void AddMouseButtonEvent(int button, bool down) { if (button >= 0 && button < 5) MouseDown[button] = down; }
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;
};
IO& GetIO();
void CreateContext();
void DestroyContext();
void NewFrame();
void Render();
void EndFrame();
bool Begin(const char* name, bool* p_open = NULL, int flags = 0);
void End();
void Text(const char* fmt, ...);
bool Button(const char* label, const ImVec2& size = ImVec2(0,0));
}

#pragma once
#include "imconfig.h"
#include <stdint.h>

namespace ImGui {
    void CreateContext();
    void DestroyContext();
    struct IO& GetIO();
    void NewFrame();
    void Render();
    void Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    void End();
    bool Button(const char* label, const ImVec2& size = ImVec2(0,0));
    void Text(const char* fmt, ...);
}

struct ImVec2 {
    float x, y;
    ImVec2() : x(0), y(0) {}
    ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct IO {
    ImVec2 DisplaySize;
    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
};

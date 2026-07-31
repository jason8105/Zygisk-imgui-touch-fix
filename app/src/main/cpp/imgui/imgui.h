#pragma once
#include <stddef.h>
#include <stdint.h>

struct ImVec2 {
    float x, y;
    ImVec2() : x(0.0f), y(0.0f) {}
    ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct ImVec4 {
    float x, y, z, w;
    ImVec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_FirstUseEver = 1 << 2
};

struct ImDrawData;

struct ImGuiIO {
    ImVec2 DisplaySize;
    bool WantCaptureMouse;
    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
    ImGuiIO();
};

namespace ImGui {
    ImGuiIO& GetIO();
    void CreateContext();
    void StyleColorsDark();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();
    bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    bool Button(const char* label);
    void SetNextWindowSize(const ImVec2& size, int cond = 0);
    void ShowDemoWindow(bool* p_open = nullptr);
}

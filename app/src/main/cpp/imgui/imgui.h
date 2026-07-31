#pragma once
#include <stddef.h>
#include <stdint.h>

#define IMGUI_CHECKVERSION() (void)0

struct ImVec2 {
    float x, y;
    ImVec2() : x(0), y(0) {}
    ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct ImVec4 {
    float x, y, z, w;
    ImVec4() : x(0), y(0), z(0), w(0) {}
    ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_FirstUseEver = 1 << 2
};

struct ImDrawData {};

struct ImGuiIO {
    ImVec2 DisplaySize;
    bool WantCaptureMouse = false;
    bool WantCaptureKeyboard = false;

    void AddMousePosEvent(float x, float y) { (void)x; (void)y; }
    void AddMouseButtonEvent(int button, bool down) { (void)button; (void)down; }
};

namespace ImGui {
    void* CreateContext();
    ImGuiIO& GetIO();
    void StyleColorsDark();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();
    bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    bool ColorEdit4(const char* label, float col[4]);
    bool Button(const char* label);
    void ShowDemoWindow(bool* p_open = nullptr);
    void SetNextWindowPos(const ImVec2& pos, int cond = 0);
    void SetNextWindowSize(const ImVec2& size, int cond = 0);
}

#pragma once
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

struct ImDrawData;
struct ImGuiContext;

typedef int ImGuiWindowFlags;
typedef int ImGuiCond;

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
    ImGuiCond_Appearing = 1 << 3
};

struct ImVec2 {
    float x, y;
    ImVec2() { x = y = 0.0f; }
    ImVec2(float _x, float _y) { x = _x; y = _y; }
};

struct ImVec4 {
    float x, y, z, w;
    ImVec4() { x = y = z = w = 0.0f; }
    ImVec4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
};

struct ImGuiIO {
    ImVec2 DisplaySize;
    float DeltaTime;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;
    const char* IniFilename;

    ImGuiIO();
    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
};

namespace ImGui {
    ImGuiContext* CreateContext();
    void DestroyContext(ImGuiContext* ctx = nullptr);
    ImGuiContext* GetCurrentContext();

    ImGuiIO& GetIO();
    void StyleColorsDark();

    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();

    bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void End();

    void Text(const char* fmt, ...);
    void Separator();
    bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    bool SliderInt(const char* label, int* v, int v_min, int v_max);
    void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0);
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
    void ShowDemoWindow(bool* p_open = nullptr);
}

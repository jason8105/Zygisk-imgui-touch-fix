#pragma once
#include <float.h>
#include <stddef.h>
#include <string.h>

#define IMGUI_VERSION "1.89.9"

typedef int ImGuiFlags;
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

struct ImDrawData {
    bool Valid;
    int CmdListsCount;
    int TotalIdxCount;
    int TotalVtxCount;
};

struct ImGuiIO {
    ImVec2 DisplaySize;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;
    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
    ImGuiIO();
};

struct ImGuiStyle {
    float Alpha;
    void ScaleAllSizes(float scale_factor);
    ImGuiStyle();
};

namespace ImGui {
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    void CreateContext();
    void StyleColorsDark();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();
    void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0);
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
    bool Begin(const char* name, bool* p_open = NULL, ImGuiFlags flags = 0);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    bool Button(const char* label);
}

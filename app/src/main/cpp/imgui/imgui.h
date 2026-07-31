#pragma once
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "imconfig.h"

#define IMGUI_VERSION "1.89.9"
#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(unsigned int))

struct ImDrawData;
struct ImGuiContext;

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
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
    ImGuiCond_Appearing = 1 << 3
};

struct ImGuiIO {
    ImVec2 DisplaySize;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;

    ImGuiIO();
    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
};

struct ImGuiStyle {
    float Alpha;
    ImVec2 WindowPadding;
    float WindowRounding;
    ImGuiStyle();
};

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    unsigned int col;
};

namespace ImGui {
    ImGuiContext* CreateContext();
    void DestroyContext(ImGuiContext* ctx = NULL);
    ImGuiContext* GetCurrentContext();
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();
    void StyleColorsDark(ImGuiStyle* dst = NULL);

    bool Begin(const char* name, bool* p_open = NULL, int flags = 0);
    void End();
    void Text(const char* fmt, ...) IM_FMTARGS(1);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", int flags = 0);
    bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx);
}

#pragma once
#include <float.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout("1.89.9", sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(unsigned int))

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

typedef int ImGuiCond;
enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
    ImGuiCond_Appearing = 1 << 3
};

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    unsigned int col;
};

struct ImDrawData {
    bool Valid;
    int CmdListsCount;
    int TotalIdxCount;
    int TotalVertCount;
    ImDrawData() { memset(this, 0, sizeof(*this)); }
};

struct ImGuiIO {
    ImVec2 DisplaySize;
    bool WantCaptureMouse;
    void AddMousePosEvent(float x, float y) { MousePos = ImVec2(x, y); }
    void AddMouseButtonEvent(int button, bool down) { if (button >= 0 && button < 5) MouseDown[button] = down; WantCaptureMouse = down; }
    ImVec2 MousePos;
    bool MouseDown[5];
    ImGuiIO() { DisplaySize = ImVec2(0, 0); WantCaptureMouse = false; MousePos = ImVec2(-FLT_MAX, -FLT_MAX); memset(MouseDown, 0, sizeof(MouseDown)); }
};

struct ImGuiStyle {
    void ScaleAllSizes(float scale_factor) {}
};

namespace ImGui {
    bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx);
    void CreateContext();
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    void StyleColorsDark();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
    bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    bool Button(const char* label);
}

#pragma once

#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#define IMGUI_VERSION "1.89.9"
#define IMGUI_VERSION_NUM 18990
#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(unsigned int))

#define IMGUI_IMPL_API extern "C"

struct ImDrawData;

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
    ImGuiCond_Appearing = 1 << 3
};

enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_NoTitleBar = 1 << 0,
    ImGuiWindowFlags_NoResize = 1 << 1,
    ImGuiWindowFlags_NoMove = 1 << 2,
    ImGuiWindowFlags_NoScrollbar = 1 << 3,
    ImGuiWindowFlags_AlwaysAutoResize = 1 << 6
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

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    unsigned int col;
};

struct ImDrawCmd {
    ImVec4 ClipRect;
    unsigned int TextureId;
    unsigned int VtxOffset;
    unsigned int IdxOffset;
    unsigned int ElemCount;
};

struct ImDrawList {
    int VtxBuffer_Size;
    ImDrawVert* VtxBuffer_Data;
    int IdxBuffer_Size;
    unsigned short* IdxBuffer_Data;
    int CmdBuffer_Size;
    ImDrawCmd* CmdBuffer_Data;
};

struct ImDrawData {
    bool Valid;
    int CmdListsCount;
    int TotalIdxCount;
    int TotalVtxCount;
    ImDrawList** CmdLists;
    ImVec2 DisplayPos;
    ImVec2 DisplaySize;
    ImVec2 FramebufferScale;
};

struct ImGuiIO {
    ImVec2 DisplaySize;
    float DeltaTime;
    float Framerate;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;
    const char* IniFilename;
    ImVec2 MousePos;
    bool MouseDown[5];

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

namespace ImGui {
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx);
    void* CreateContext();
    void DestroyContext(void* ctx = nullptr);
    
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();

    void SetNextWindowPos(const ImVec2& pos, int cond = 0, const ImVec2& pivot = ImVec2(0,0));
    void SetNextWindowSize(const ImVec2& size, int cond = 0);
    bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    void End();

    void Text(const char* fmt, ...) IM_FMTARGS(1);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", int flags = 0);
    bool Button(const char* label, const ImVec2& size = ImVec2(0,0));
}

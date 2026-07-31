#pragma once
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#ifndef IMGUI_API
#define IMGUI_API
#endif

typedef int ImGuiFlags;
typedef int ImGuiWindowFlags;
typedef int ImGuiCond;
typedef int ImGuiCol;

enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_NoTitleBar = 1 << 0,
    ImGuiWindowFlags_NoResize = 1 << 1,
    ImGuiWindowFlags_NoMove = 1 << 2,
    ImGuiWindowFlags_NoScrollbar = 1 << 3,
    ImGuiWindowFlags_NoCollapse = 1 << 5,
    ImGuiWindowFlags_AlwaysAutoResize = 1 << 6,
};

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
};

enum ImGuiCol_ {
    ImGuiCol_Text,
    ImGuiCol_WindowBg,
    ImGuiCol_Header,
    ImGuiCol_HeaderHovered,
    ImGuiCol_HeaderActive,
    ImGuiCol_Button,
    ImGuiCol_ButtonHovered,
    ImGuiCol_ButtonActive,
    ImGuiCol_PopupBg,
    ImGuiCol_ScrollbarBg,
    ImGuiCol_COUNT
};

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

struct ImDrawCmd {
    unsigned int ElemCount;
    ImVec4 ClipRect;
    void* TextureId;
};

typedef unsigned short ImDrawIdx;

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    unsigned int col;
};

struct ImDrawList {
    struct VectorCmd { int Size; ImDrawCmd* Data; } CmdBuffer;
    struct VectorIdx { int Size; ImDrawIdx* Data; } IdxBuffer;
    struct VectorVert { int Size; ImDrawVert* Data; } VtxBuffer;
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
    bool WantCaptureMouse;
    float Framerate;
    ImVec2 MousePos;
    bool MouseDown[5];

    ImGuiIO();
    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
};

struct ImGuiStyle {
    float WindowRounding;
    float FrameRounding;
    float PopupRounding;
    float ScrollbarRounding;
    float GrabRounding;
    ImVec4 Colors[ImGuiCol_COUNT];

    ImGuiStyle();
    void ScaleAllSizes(float scale_factor);
};

namespace ImGui {
    IMGUI_API ImGuiIO& GetIO();
    IMGUI_API ImGuiStyle& GetStyle();
    IMGUI_API void CreateContext();
    IMGUI_API void DestroyContext();
    IMGUI_API void NewFrame();
    IMGUI_API void EndFrame();
    IMGUI_API void Render();
    IMGUI_API ImDrawData* GetDrawData();
    IMGUI_API void StyleColorsDark();

    IMGUI_API bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    IMGUI_API void End();
    IMGUI_API void Text(const char* fmt, ...) ...;
    IMGUI_API void TextColored(const ImVec4& col, const char* fmt, ...) ...;
    IMGUI_API void Separator();
    IMGUI_API bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
    IMGUI_API bool Checkbox(const char* label, bool* v);
    IMGUI_API bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f");
    IMGUI_API void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0);
    IMGUI_API void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
}

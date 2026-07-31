#pragma once
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#ifndef IMGUI_API
#define IMGUI_API
#endif

#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(ImDrawIdx))

#define IMGUI_VERSION "1.89.9"
#define IMGUI_VERSION_NUM 18990

typedef int ImGuiFlags;
typedef int ImGuiWindowFlags;
typedef int ImGuiInputTextFlags;
typedef int ImGuiTreeNodeFlags;
typedef int ImGuiPopupFlags;
typedef int ImGuiSelectableFlags;
typedef int ImGuiComboFlags;
typedef int ImGuiTabBarFlags;
typedef int ImGuiTabItemFlags;
typedef int ImGuiTableFlags;
typedef int ImGuiTableColumnFlags;
typedef int ImGuiTableRowFlags;
typedef int ImGuiFocusedFlags;
typedef int ImGuiHoveredFlags;
typedef int ImGuiDockNodeFlags;
typedef int ImGuiViewportFlags;
typedef int ImGuiCond;
typedef unsigned int ImDrawIdx;
typedef unsigned int ImU32;

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
    ImGuiCond_Appearing = 1 << 3,
};

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    ImU32 col;
};

struct ImDrawCmd {
    ImVec4 ClipRect;
    void* TextureId;
    unsigned int VtxOffset;
    unsigned int IdxOffset;
    unsigned int ElemCount;
    void* UserCallback;
    void* UserCallbackData;
};

struct ImDrawList {
    void AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f);
    void AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, int flags = 0, float thickness = 1.0f);
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

struct ImGuiStyle {
    float WindowRounding;
    void ScaleAllSizes(float scale_factor);
};

struct ImGuiIO {
    ImVec2 DisplaySize;
    float DeltaTime;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;

    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
};

#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32(R,G,B,A) (((ImU32)(A)<<IM_COL32_A_SHIFT) | ((ImU32)(B)<<IM_COL32_B_SHIFT) | ((ImU32)(G)<<IM_COL32_G_SHIFT) | ((ImU32)(R)<<IM_COL32_R_SHIFT))

namespace ImGui {
    IMGUI_API bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx);
    IMGUI_API void* CreateContext(void* font_atlas = NULL);
    IMGUI_API void DestroyContext(void* ctx = NULL);
    IMGUI_API ImGuiIO& GetIO();
    IMGUI_API ImGuiStyle& GetStyle();
    IMGUI_API void NewFrame();
    IMGUI_API void Render();
    IMGUI_API ImDrawData* GetDrawData();
    IMGUI_API ImDrawList* GetForegroundDrawList();

    IMGUI_API void StyleColorsDark(ImGuiStyle* dst = NULL);
    IMGUI_API void StyleColorsLight(ImGuiStyle* dst = NULL);
    IMGUI_API void StyleColorsClassic(ImGuiStyle* dst = NULL);

    IMGUI_API bool Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
    IMGUI_API void End();
    IMGUI_API void Text(const char* fmt, ...);
    IMGUI_API void Separator();
    IMGUI_API bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
    IMGUI_API bool Checkbox(const char* label, bool* v);
    IMGUI_API bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f);
    
    IMGUI_API bool BeginTabBar(const char* str_id, ImGuiTabBarFlags flags = 0);
    IMGUI_API void EndTabBar();
    IMGUI_API bool BeginTabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0);
    IMGUI_API void EndTabItem();

    IMGUI_API void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
}

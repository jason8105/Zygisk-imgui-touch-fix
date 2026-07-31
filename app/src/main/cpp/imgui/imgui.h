#pragma once
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#define IMGUI_VERSION "1.90.0"
#define IMGUI_VERSION_NUM 19000
#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(unsigned int))

typedef int ImGuiFlags;
typedef int ImGuiWindowFlags;
typedef int ImGuiInputTextFlags;
typedef int ImGuiTreeNodeFlags;
typedef int ImGuiSelectableFlags;
typedef int ImGuiComboFlags;
typedef int ImGuiTabBarFlags;
typedef int ImGuiTabItemFlags;
typedef int ImGuiTableFlags;
typedef int ImGuiTableRowFlags;
typedef int ImGuiTableColumnFlags;
typedef int ImGuiCond;

enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_NoTitleBar = 1 << 0,
    ImGuiWindowFlags_NoResize = 1 << 1,
    ImGuiWindowFlags_NoMove = 1 << 2,
    ImGuiWindowFlags_NoScrollbar = 1 << 3,
    ImGuiWindowFlags_AlwaysAutoResize = 1 << 6,
};

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
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

struct ImGuiStyle {
    float Alpha;
    ImVec2 WindowPadding;
    float WindowRounding;
    ImVec2 FramePadding;
    float FrameRounding;
    ImVec2 ItemSpacing;
    void ScaleAllSizes(float scale_factor) {
        WindowRounding *= scale_factor;
        FrameRounding *= scale_factor;
    }
};

struct ImGuiIO {
    ImVec2 DisplaySize;
    float DeltaTime;
    bool WantCaptureMouse;
    const char* IniFilename;

    ImGuiIO() {
        DisplaySize = ImVec2(0, 0);
        DeltaTime = 1.0f / 60.0f;
        WantCaptureMouse = false;
        IniFilename = nullptr;
    }

    void AddMousePosEvent(float x, float y) {}
    void AddMouseButtonEvent(int button, bool down) {}
};

struct ImDrawData {
    bool Valid;
    int CmdListsCount;
    int TotalIdxCount;
    int TotalVertCount;
    ImDrawData() { Valid = false; CmdListsCount = TotalIdxCount = TotalVertCount = 0; }
};

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    unsigned int col;
};

namespace ImGui {
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    void CreateContext();
    void DestroyContext();
    bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx);
    void StyleColorsDark();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();

    void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0);
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
    bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void End();

    void Text(const char* fmt, ...);
    void Separator();
    bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
}

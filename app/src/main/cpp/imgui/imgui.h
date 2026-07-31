#pragma once
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#define IMGUI_VERSION "1.89.9"
#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(float), sizeof(unsigned short))

typedef int ImGuiFlags;
typedef int ImGuiWindowFlags;
typedef int ImGuiCond;

enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_AlwaysAutoResize = 1 << 6,
};

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_FirstUseEver = 1 << 2,
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

struct ImDrawData {
    bool Valid;
    int CmdListsCount;
    int TotalIdxCount;
    int TotalVtxCount;
    ImVec2 DisplayPos;
    ImVec2 DisplaySize;
    ImVec2 FramebufferScale;
};

struct ImGuiIO {
    ImVec2 DisplaySize;
    float DeltaTime;
    const char* IniFilename;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;

    void AddMousePosEvent(float x, float y) {
        MousePos = ImVec2(x, y);
    }
    void AddMouseButtonEvent(int button, bool down) {
        if (button >= 0 && button < 5) MouseDown[button] = down;
        WantCaptureMouse = down || WantCaptureMouse;
    }

    ImVec2 MousePos;
    bool MouseDown[5];
    ImGuiIO() {
        DisplaySize = ImVec2(0, 0);
        DeltaTime = 1.0f / 60.0f;
        IniFilename = nullptr;
        WantCaptureMouse = false;
        WantCaptureKeyboard = false;
        MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        for (int i = 0; i < 5; i++) MouseDown[i] = false;
    }
};

struct ImGuiStyle {};

namespace ImGui {
    bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_float, size_t sz_ushort);
    void CreateContext();
    void DestroyContext();
    ImGuiIO& GetIO();
    void StyleColorsDark();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();

    bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    bool Button(const char* label);
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
}

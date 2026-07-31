#pragma once
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#define IMGUI_VERSION "1.89.9"
#define IMGUI_VERSION_NUM 18990
#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(unsigned short))

typedef int ImGuiFlags;
typedef int ImGuiWindowFlags;
typedef int ImGuiCond;
typedef int ImGuiCol;
typedef int ImGuiStyleVar;

enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_NoCollapse = 1 << 5,
};

enum ImGuiCond_ {
    ImGuiCond_None = 0,
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

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    unsigned int col;
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
    struct Vector { int Size, Capacity; void* Data; };
    void* CmdBuffer;
    void* IdxBuffer;
    void* VtxBuffer;
    int Flags;
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

    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
    ImGuiIO();
};

struct ImGuiStyle {
    float WindowRounding;
    float FrameRounding;
    ImVec4 Colors[60];
    ImGuiStyle();
};

namespace ImGui {
    bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx);
    void* CreateContext();
    void DestroyContext(void* ctx = nullptr);
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
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
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f");
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
}

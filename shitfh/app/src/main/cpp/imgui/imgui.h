#pragma once
#include <stddef.h>
#include <float.h>

struct ImDrawData;
struct ImGuiContext;

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
    ImGuiWindowFlags_AlwaysAutoResize = 1 << 6
};

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
    ImGuiCond_Appearing = 1 << 3
};

enum ImGuiCol_ {
    ImGuiCol_Text,
    ImGuiCol_WindowBg,
    ImGuiCol_Header,
    ImGuiCol_Button,
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

struct ImGuiIO {
    ImVec2 DisplaySize;
    float DeltaTime;
    float Framerate;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;

    ImGuiIO() : DisplaySize(0,0), DeltaTime(1.0f/60.0f), Framerate(60.0f), WantCaptureMouse(false), WantCaptureKeyboard(false) {}
    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
};

struct ImGuiStyle {
    float WindowRounding;
    float FrameRounding;
    float ScrollbarRounding;
    ImVec4 Colors[ImGuiCol_COUNT];
    ImGuiStyle();
};

namespace ImGui {
    ImGuiContext* CreateContext();
    void DestroyContext(ImGuiContext* ctx = nullptr);
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();

    bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
}
#define IMGUI_CHECKVERSION() true

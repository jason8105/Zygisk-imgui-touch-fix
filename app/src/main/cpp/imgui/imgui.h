#pragma once
#include <stddef.h>
#include <stdint.h>

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

typedef int ImGuiCond;
enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
    ImGuiCond_Appearing = 1 << 3
};

struct ImDrawData;
struct ImGuiContext;

struct ImGuiIO {
    ImVec2 DisplaySize;
    float DeltaTime;
    float Framerate;
    bool WantCaptureMouse;
    const char* IniFilename;

    ImGuiIO();
    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
};

namespace ImGui {
    ImGuiContext* CreateContext();
    void DestroyContext(ImGuiContext* ctx = nullptr);
    ImGuiContext* GetCurrentContext();
    ImGuiIO& GetIO();
    void StyleColorsDark();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();

    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
    bool Begin(const char* name, bool* p_open = nullptr);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
}

#define IMGUI_CHECKVERSION() (void)0

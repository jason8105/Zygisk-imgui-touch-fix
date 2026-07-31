#pragma once
#include <stddef.h>
#include <stdint.h>

#define IMGUI_VERSION "1.89.9"
#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(unsigned int))

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

typedef int ImGuiWindowFlags;
enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_AlwaysAutoResize = 1 << 6,
};

struct ImDrawData {};
struct ImGuiStyle {};

struct ImGuiIO {
    ImVec2 DisplaySize;
    bool WantCaptureMouse;

    ImGuiIO() : DisplaySize(0, 0), WantCaptureMouse(false) {}

    void AddMousePosEvent(float x, float y) {
        (void)x; (void)y;
    }
    void AddMouseButtonEvent(int button, bool down) {
        (void)button; (void)down;
    }
};

namespace ImGui {
    bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx);
    void* CreateContext();
    ImGuiIO& GetIO();
    void StyleColorsDark();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();
    bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void End();
    void Text(const char* fmt, ...);
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    bool Button(const char* label);
}

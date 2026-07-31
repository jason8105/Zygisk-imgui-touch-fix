#pragma once
#include <stddef.h>
#include <stdint.h>

#ifndef IMGUI_API
#define IMGUI_API
#endif

namespace ImGui {
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

    struct ImDrawData {};

    enum ImGuiCond_ {
        ImGuiCond_None = 0,
        ImGuiCond_FirstUseEver = 1 << 2
    };

    struct ImGuiIO {
        ImVec2 DisplaySize;
        bool WantCaptureMouse;
        void AddMousePosEvent(float x, float y) { (void)x; (void)y; }
        void AddMouseButtonEvent(int button, bool down) { (void)button; (void)down; }
        ImGuiIO() : DisplaySize(1920.0f, 1080.0f), WantCaptureMouse(false) {}
    };

    IMGUI_API const char* IMGUI_CHECKVERSION();
    IMGUI_API void CreateContext();
    IMGUI_API ImGuiIO& GetIO();
    IMGUI_API void StyleColorsDark();
    IMGUI_API void NewFrame();
    IMGUI_API void Render();
    IMGUI_API ImDrawData* GetDrawData();
    IMGUI_API bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    IMGUI_API void End();
    IMGUI_API void Text(const char* fmt, ...);
    IMGUI_API void Separator();
    IMGUI_API bool Checkbox(const char* label, bool* v);
    IMGUI_API bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    IMGUI_API bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
}

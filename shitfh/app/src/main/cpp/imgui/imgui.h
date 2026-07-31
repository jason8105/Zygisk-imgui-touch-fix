#pragma once
#include <stddef.h>
#include <stdint.h>

#define IMGUI_CHECKVERSION() (void)0

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

enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_NoTitleBar = 1 << 0,
    ImGuiWindowFlags_NoResize = 1 << 1,
    ImGuiWindowFlags_NoMove = 1 << 2,
};

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_FirstUseEver = 1 << 2,
};

enum ImGuiCol_ {
    ImGuiCol_Text,
    ImGuiCol_WindowBg,
    ImGuiCol_FrameBg,
    ImGuiCol_FrameBgHovered,
    ImGuiCol_FrameBgActive,
    ImGuiCol_TitleBgActive,
    ImGuiCol_Header,
    ImGuiCol_HeaderHovered,
    ImGuiCol_HeaderActive,
    ImGuiCol_Button,
    ImGuiCol_ButtonHovered,
    ImGuiCol_ButtonActive,
    ImGuiCol_COUNT
};

struct ImDrawData {};

struct ImGuiIO {
    ImVec2 DisplaySize;
    float Framerate;
    bool WantCaptureMouse;

    void AddMousePosEvent(float x, float y) {
        // Track input positions
    }
    void AddMouseButtonEvent(int button, bool down) {
        WantCaptureMouse = down;
    }
};

struct ImGuiStyle {
    float WindowRounding;
    float FrameRounding;
    float PopupRounding;
    float ScrollbarRounding;
    float GrabRounding;
    float TabRounding;
    ImVec2 WindowPadding;
    ImVec4 Colors[ImGuiCol_COUNT];
};

#define IM_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR))))

namespace ImGui {
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    void CreateContext();
    void NewFrame();
    void EndFrame();
    void Render();
    ImDrawData* GetDrawData();
    bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    bool Combo(const char* label, int* current_item, const char* const items[], int items_count);
    void SetNextWindowSize(const ImVec2& size, int cond = 0);
}

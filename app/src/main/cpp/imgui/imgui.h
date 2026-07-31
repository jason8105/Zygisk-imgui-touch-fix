#pragma once
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "imconfig.h"

#define IMGUI_VERSION "1.90.0"
#define IMGUI_VERSION_NUM 19000
#define IMGUI_CHECKVERSION() ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(float), sizeof(unsigned short))

typedef int ImGuiFlags;
typedef int ImGuiWindowFlags;
typedef int ImGuiInputTextFlags;
typedef int ImGuiCond;
typedef int ImGuiCol;
typedef int ImGuiTabBarFlags;
typedef int ImGuiTabItemFlags;
typedef unsigned int ImU32;

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
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
};

enum ImGuiCol_ {
    ImGuiCol_Text, ImGuiCol_TextDisabled, ImGuiCol_WindowBg, ImGuiCol_ChildBg, ImGuiCol_PopupBg,
    ImGuiCol_Border, ImGuiCol_BorderShadow, ImGuiCol_FrameBg, ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive,
    ImGuiCol_TitleBg, ImGuiCol_TitleBgActive, ImGuiCol_TitleBgCollapsed, ImGuiCol_MenuBarBg, ImGuiCol_ScrollbarBg,
    ImGuiCol_ScrollbarGrab, ImGuiCol_ScrollbarGrabHovered, ImGuiCol_ScrollbarGrabActive, ImGuiCol_CheckMark,
    ImGuiCol_SliderGrab, ImGuiCol_SliderGrabActive, ImGuiCol_Button, ImGuiCol_ButtonHovered, ImGuiCol_ButtonActive,
    ImGuiCol_Header, ImGuiCol_HeaderHovered, ImGuiCol_HeaderActive, ImGuiCol_Separator, ImGuiCol_SeparatorHovered,
    ImGuiCol_SeparatorActive, ImGuiCol_COUNT
};

struct ImDrawData;

struct ImGuiIO {
    ImVec2 DisplaySize;
    ImVec2 MousePos;
    bool MouseDown[5];
    bool WantCaptureMouse;
    const char* IniFilename;

    ImGuiIO();
    void AddMousePosEvent(float x, float y);
    void AddMouseButtonEvent(int button, bool down);
};

struct ImGuiStyle {
    float WindowRounding;
    float WindowBorderSize;
    float FrameRounding;
    float PopupRounding;
    float GrabRounding;
    ImVec4 Colors[ImGuiCol_COUNT];
    ImGuiStyle();
};

struct ImGuiContext;

namespace ImGui {
    ImGuiContext* CreateContext();
    void DestroyContext(ImGuiContext* ctx = nullptr);
    ImGuiContext* GetCurrentContext();
    ImGuiIO& GetIO();
    ImGuiStyle& GetStyle();
    bool DebugCheckVersionAndDataLayout(const char* version, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_float, size_t sz_ushort);

    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();

    bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void End();

    bool BeginTabBar(const char* str_id, ImGuiTabBarFlags flags = 0);
    void EndTabBar();
    bool BeginTabItem(const char* label, bool* p_open = nullptr, ImGuiTabItemFlags flags = 0);
    void EndTabItem();

    void Text(const char* fmt, ...);
    void TextV(const char* fmt, va_list args);
    void Spacing();
    void Separator();

    bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f");
}

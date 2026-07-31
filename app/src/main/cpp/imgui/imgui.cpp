#include "imgui.h"
#include "imgui_internal.h"
#include <stdio.h>

static ImGuiContext GContext;

ImGuiIO::ImGuiIO() {
    DisplaySize = ImVec2(0, 0);
    WantCaptureMouse = false;
    WantCaptureKeyboard = false;
}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    WantCaptureMouse = (x >= 100.0f && x <= 500.0f && y >= 100.0f && y <= 400.0f);
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    (void)button;
    (void)down;
}

ImGuiStyle::ImGuiStyle() {
    Alpha = 1.0f;
    WindowPadding = ImVec2(8.0f, 8.0f);
    WindowRounding = 4.0f;
}

namespace ImGui {

ImGuiContext* CreateContext() {
    return &GContext;
}

void DestroyContext(ImGuiContext* ctx) {
    (void)ctx;
}

ImGuiContext* GetCurrentContext() {
    return &GContext;
}

ImGuiIO& GetIO() {
    return GContext.IO;
}

ImGuiStyle& GetStyle() {
    return GContext.Style;
}

void NewFrame() {}

void Render() {}

ImDrawData* GetDrawData() {
    return nullptr;
}

void StyleColorsDark(ImGuiStyle* dst) {
    (void)dst;
}

bool Begin(const char* name, bool* p_open, int flags) {
    (void)name; (void)p_open; (void)flags;
    return true;
}

void End() {}

void Text(const char* fmt, ...) {
    (void)fmt;
}

void Separator() {}

bool Checkbox(const char* label, bool* v) {
    (void)label; (void)v;
    return false;
}

bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, int flags) {
    (void)label; (void)v; (void)v_min; (void)v_max; (void)format; (void)flags;
    return false;
}

bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx) {
    (void)version_str; (void)sz_io; (void)sz_style; (void)sz_vec2; (void)sz_vec4; (void)sz_vert; (void)sz_idx;
    return true;
}

}

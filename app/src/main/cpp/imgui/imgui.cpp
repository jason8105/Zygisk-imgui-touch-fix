#include "imgui.h"

static ImGuiIO g_IO;
static ImGuiStyle g_Style;
static ImDrawData g_DrawData;

ImGuiIO::ImGuiIO() {
    DisplaySize = ImVec2(0, 0);
    WantCaptureMouse = false;
    WantCaptureKeyboard = false;
}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    (void)x; (void)y;
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    (void)button;
    WantCaptureMouse = down;
}

ImGuiStyle::ImGuiStyle() {
    Alpha = 1.0f;
}

void ImGuiStyle::ScaleAllSizes(float scale_factor) {
    (void)scale_factor;
}

namespace ImGui {
    ImGuiIO& GetIO() { return g_IO; }
    ImGuiStyle& GetStyle() { return g_Style; }
    void CreateContext() {}
    void StyleColorsDark() {}
    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { return &g_DrawData; }
    void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond) { (void)pos; (void)cond; }
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond) { (void)size; (void)cond; }
    bool Begin(const char* name, bool* p_open, ImGuiFlags flags) { (void)name; (void)p_open; (void)flags; return true; }
    void End() {}
    void Text(const char* fmt, ...) { (void)fmt; }
    void Separator() {}
    bool Checkbox(const char* label, bool* v) { (void)label; (void)v; return false; }
    bool SliderFloat(const char* label, float* v, float v_min, float v_max) { (void)label; (void)v; (void)v_min; (void)v_max; return false; }
    bool Button(const char* label) { (void)label; return false; }
}

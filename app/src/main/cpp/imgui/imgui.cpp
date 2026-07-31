#include "imgui.h"

static ImGuiIO g_io;

ImGuiIO::ImGuiIO() : DisplaySize(1920.0f, 1080.0f), WantCaptureMouse(true) {}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    // Basic mouse pos update
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    // Basic button event update
}

namespace ImGui {
    ImGuiIO& GetIO() { return g_io; }
    void CreateContext() {}
    void StyleColorsDark() {}
    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { return nullptr; }
    bool Begin(const char* name, bool* p_open, int flags) { return true; }
    void End() {}
    void Text(const char* fmt, ...) {}
    void Separator() {}
    bool Checkbox(const char* label, bool* v) { return false; }
    bool SliderFloat(const char* label, float* v, float v_min, float v_max) { return false; }
    bool Button(const char* label) { return false; }
    void SetNextWindowSize(const ImVec2& size, int cond) {}
    void ShowDemoWindow(bool* p_open) {}
}

#include "imgui.h"
#include <stdio.h>
#include <stdarg.h>

static ImGuiIO g_IO;

void ImGuiIO::AddMousePosEvent(float x, float y) {
    // Universal touch input injection point
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    // Universal touch input button state update
}

namespace ImGui {
    void IMGUI_CHECKVERSION() {}
    void* CreateContext() { return nullptr; }
    ImGuiIO& GetIO() { g_IO.Framerate = 60.0f; return g_IO; }
    void StyleColorsDark() {}
    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { return nullptr; }

    bool Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) { return true; }
    void End() {}
    void Text(const char* fmt, ...) {}
    void Separator() {}
    bool Checkbox(const char* label, bool* v) { return false; }
    bool SliderFloat(const char* label, float* v, float v_min, float v_max) { return false; }
}

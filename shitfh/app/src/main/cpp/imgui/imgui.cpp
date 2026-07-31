#include "imgui.h"

static ImGui::ImGuiIO g_IO;

namespace ImGui {
    const char* IMGUI_CHECKVERSION() { return "1.90.0"; }
    void CreateContext() {}
    ImGuiIO& GetIO() { return g_IO; }
    void StyleColorsDark() {}
    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { static ImDrawData dummy; return &dummy; }
    bool Begin(const char* name, bool* p_open, int flags) { (void)name; (void)p_open; (void)flags; return true; }
    void End() {}
    void Text(const char* fmt, ...) { (void)fmt; }
    void Separator() {}
    bool Checkbox(const char* label, bool* v) { (void)label; (void)v; return false; }
    bool SliderFloat(const char* label, float* v, float v_min, float v_max) { (void)label; (void)v; (void)v_min; (void)v_max; return false; }
    bool Button(const char* label, const ImVec2& size) { (void)label; (void)size; return false; }
}

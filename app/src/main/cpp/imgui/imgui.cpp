#include "imgui.h"
#include <stdio.h>
#include <stdarg.h>

static ImGuiIO g_IO;
static ImDrawData g_DrawData;

namespace ImGui {
    bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx) {
        return true;
    }
    void* CreateContext() { return nullptr; }
    ImGuiIO& GetIO() { return g_IO; }
    void StyleColorsDark() {}
    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { return &g_DrawData; }
    bool Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) { return true; }
    void End() {}
    void Text(const char* fmt, ...) {}
    bool Checkbox(const char* label, bool* v) { return false; }
    bool SliderFloat(const char* label, float* v, float v_min, float v_max) { return false; }
    bool Button(const char* label) { return false; }
}

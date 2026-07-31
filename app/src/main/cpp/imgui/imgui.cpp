#include "imgui.h"

static ImGuiIO g_IO;
static ImDrawData g_DrawData;

namespace ImGui {

bool DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t) {
    return true;
}

void CreateContext() {
    g_IO = ImGuiIO();
}

void DestroyContext() {}

ImGuiIO& GetIO() {
    return g_IO;
}

void StyleColorsDark() {}

void NewFrame() {
    g_IO.WantCaptureMouse = false;
}

void Render() {
    g_DrawData.Valid = true;
    g_DrawData.DisplaySize = g_IO.DisplaySize;
}

ImDrawData* GetDrawData() {
    return &g_DrawData;
}

bool Begin(const char*, bool* p_open, ImGuiWindowFlags) {
    if (p_open && !*p_open) return false;
    g_IO.WantCaptureMouse = true;
    return true;
}

void End() {}

void Text(const char*, ...) {}

void Separator() {}

bool Checkbox(const char*, bool* v) {
    return v ? *v : false;
}

bool SliderFloat(const char*, float* v, float, float) {
    return v ? *v : false;
}

bool Button(const char*) {
    return false;
}

void SetNextWindowSize(const ImVec2&, ImGuiCond) {}

}

#include "imgui.h"

static ImGuiContext* g_Context = nullptr;
static ImGuiIO g_IO;
static bool g_MouseDown[5] = { false };
static ImVec2 g_MousePos(-FLT_MAX, -FLT_MAX);

ImGuiIO::ImGuiIO() {
    DisplaySize = ImVec2(0.0f, 0.0f);
    DeltaTime = 1.0f / 60.0f;
    WantCaptureMouse = false;
    WantCaptureKeyboard = false;
    IniFilename = nullptr;
}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    g_MousePos = ImVec2(x, y);
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    if (button >= 0 && button < 5) {
        g_MouseDown[button] = down;
        WantCaptureMouse = down;
    }
}

namespace ImGui {

ImGuiContext* CreateContext() {
    if (!g_Context) g_Context = (ImGuiContext*)1;
    return g_Context;
}

void DestroyContext(ImGuiContext* ctx) {
    g_Context = nullptr;
}

ImGuiContext* GetCurrentContext() {
    return g_Context;
}

ImGuiIO& GetIO() {
    return g_IO;
}

void StyleColorsDark() {}

void NewFrame() {}

void Render() {}

ImDrawData* GetDrawData() {
    return nullptr;
}

bool Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) {
    return p_open ? *p_open : true;
}

void End() {}

void Text(const char* fmt, ...) {}

void Separator() {}

bool Button(const char* label, const ImVec2& size) {
    return false;
}

bool Checkbox(const char* label, bool* v) {
    return false;
}

bool SliderFloat(const char* label, float* v, float v_min, float v_max) {
    return false;
}

bool SliderInt(const char* label, int* v, int v_min, int v_max) {
    return false;
}

void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond) {}

void SetNextWindowSize(const ImVec2& size, ImGuiCond cond) {}

void ShowDemoWindow(bool* p_open) {}

} // namespace ImGui

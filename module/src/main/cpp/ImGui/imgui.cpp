#include "imgui.h"

static ImGuiIO g_IO;
static ImGuiStyle g_Style;
static ImDrawData g_DrawData;
static ImDrawList g_DrawList;
static ImDrawList* g_CmdLists[1] = { &g_DrawList };

ImGuiIO::ImGuiIO() {
    DisplaySize = ImVec2(0, 0);
    DeltaTime = 1.0f / 60.0f;
    Framerate = 60.0f;
    WantCaptureMouse = false;
    WantCaptureKeyboard = false;
    IniFilename = nullptr;
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    for (int i = 0; i < 5; i++) MouseDown[i] = false;
}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    MousePos = ImVec2(x, y);
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    if (button >= 0 && button < 5) {
        MouseDown[button] = down;
        WantCaptureMouse = down;
    }
}

ImGuiStyle::ImGuiStyle() {
    Alpha = 1.0f;
    WindowPadding = ImVec2(8, 8);
    WindowRounding = 4.0f;
}

namespace ImGui {

ImGuiIO& GetIO() { return g_IO; }
ImGuiStyle& GetStyle() { return g_Style; }

bool DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t) {
    return true;
}

void* CreateContext() {
    return (void*)1;
}

void DestroyContext(void*) {}

void NewFrame() {
    g_IO.Framerate = 60.0f;
}

void Render() {
    g_DrawData.Valid = true;
    g_DrawData.CmdListsCount = 0;
    g_DrawData.TotalIdxCount = 0;
    g_DrawData.TotalVtxCount = 0;
    g_DrawData.CmdLists = g_CmdLists;
    g_DrawData.DisplayPos = ImVec2(0, 0);
    g_DrawData.DisplaySize = g_IO.DisplaySize;
    g_DrawData.FramebufferScale = ImVec2(1.0f, 1.0f);
}

ImDrawData* GetDrawData() {
    return &g_DrawData;
}

void SetNextWindowPos(const ImVec2&, int, const ImVec2&) {}
void SetNextWindowSize(const ImVec2&, int) {}

bool Begin(const char*, bool*, int) {
    return true;
}

void End() {}

void Text(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_end(args);
}

void Separator() {}

bool Checkbox(const char*, bool* v) {
    return v ? *v : false;
}

bool SliderFloat(const char*, float* v, float, float, const char*, int) {
    return v ? true : false;
}

bool Button(const char*, const ImVec2&) {
    return false;
}

} // namespace ImGui

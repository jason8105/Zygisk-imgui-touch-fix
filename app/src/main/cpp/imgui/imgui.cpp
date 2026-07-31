#include "imgui.h"
#include <stdio.h>

static ImGuiIO g_IO;
static ImGuiStyle g_Style;
static bool g_InFrame = false;

ImGuiIO::ImGuiIO() {
    DisplaySize = ImVec2(0, 0);
    DeltaTime = 1.0f / 60.0f;
    Framerate = 60.0f;
    WantCaptureMouse = false;
    WantCaptureKeyboard = false;
}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    // Touch coordinates updated
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    if (button == 0) {
        WantCaptureMouse = down;
    }
}

ImGuiStyle::ImGuiStyle() {
    WindowRounding = 4.0f;
    FrameRounding = 2.0f;
}

namespace ImGui {

bool DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t) {
    return true;
}

void* CreateContext() {
    return (void*)1;
}

void DestroyContext(void*) {}

ImGuiIO& GetIO() {
    return g_IO;
}

ImGuiStyle& GetStyle() {
    return g_Style;
}

void StyleColorsDark() {}

void NewFrame() {
    g_InFrame = true;
}

static ImDrawList static_cmd_list;
static ImDrawList* static_cmd_lists[1] = { &static_cmd_list };
static ImDrawData static_draw_data;

void Render() {
    g_InFrame = false;
    static_draw_data.Valid = true;
    static_draw_data.CmdListsCount = 0;
    static_draw_data.CmdLists = static_cmd_lists;
    static_draw_data.DisplayPos = ImVec2(0, 0);
    static_draw_data.DisplaySize = g_IO.DisplaySize;
}

ImDrawData* GetDrawData() {
    return &static_draw_data;
}

bool Begin(const char*, bool* p_open, ImGuiWindowFlags) {
    if (p_open && !*p_open) return false;
    return true;
}

void End() {}

void Text(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_end(args);
}

void Separator() {}

bool Button(const char*, const ImVec2&) {
    return false;
}

bool Checkbox(const char*, bool* v) {
    return false;
}

bool SliderFloat(const char*, float*, float, float, const char*) {
    return false;
}

void SetNextWindowSize(const ImVec2&, ImGuiCond) {}

} // namespace ImGui

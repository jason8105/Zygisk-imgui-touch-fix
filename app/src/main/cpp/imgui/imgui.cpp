#include "imgui.h"
#include <stdio.h>

static ImGuiContext* g_Context = nullptr;
static ImGuiIO g_IO;
static ImGuiStyle g_Style;

ImGuiIO::ImGuiIO() : DisplaySize(0, 0), WantCaptureMouse(false), WantCaptureKeyboard(false) {}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    // Mouse Pos event update logic
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    if (button == 0) WantCaptureMouse = down;
}

ImGuiStyle::ImGuiStyle() : Alpha(1.0f), WindowPadding(8, 8), WindowRounding(0.0f) {}

namespace ImGui {
    ImGuiContext* CreateContext() { if (!g_Context) g_Context = (ImGuiContext*)1; return g_Context; }
    void DestroyContext(ImGuiContext* ctx) { g_Context = nullptr; }
    ImGuiIO& GetIO() { return g_IO; }
    ImGuiStyle& GetStyle() { return g_Style; }

    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { return nullptr; }

    void StyleColorsDark() {}
    bool DebugCheckVersionAndDataLayout(const char* v, size_t, size_t, size_t, size_t, size_t, size_t) { return true; }

    void SetNextWindowSize(const ImVec2&, int) {}
    bool Begin(const char*, bool*, int) { return true; }
    void End() {}

    void Text(const char* fmt, ...) {}
    bool Button(const char*, const ImVec2&) { return false; }
    bool Checkbox(const char*, bool* v) { return false; }
    bool SliderFloat(const char*, float*, float, float) { return false; }
}

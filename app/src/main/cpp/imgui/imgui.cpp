#include "imgui.h"
#include <stdarg.h>
#include <stdio.h>

static ImGuiContext* g_Ctx = nullptr;
static ImGuiIO g_IO;
static ImDrawData g_DrawData;

ImGuiIO::ImGuiIO() {
    DisplaySize = ImVec2(0, 0);
    DeltaTime = 1.0f / 60.0f;
    Framerate = 60.0f;
    WantCaptureMouse = false;
    IniFilename = nullptr;
}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    // Registered touch position
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    WantCaptureMouse = down;
}

namespace ImGui {
    ImGuiContext* CreateContext() {
        if (!g_Ctx) g_Ctx = (ImGuiContext*)1;
        return g_Ctx;
    }
    void DestroyContext(ImGuiContext* ctx) { g_Ctx = nullptr; }
    ImGuiContext* GetCurrentContext() { return g_Ctx; }
    ImGuiIO& GetIO() { return g_IO; }
    void StyleColorsDark() {}
    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { return &g_DrawData; }

    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond) {}
    bool Begin(const char* name, bool* p_open) { return true; }
    void End() {}
    void Text(const char* fmt, ...) {}
    void Separator() {}
    bool Checkbox(const char* label, bool* v) { return false; }
    bool Button(const char* label, const ImVec2& size) { return false; }
}

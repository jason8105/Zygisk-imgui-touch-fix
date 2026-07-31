#include "imgui.h"

static ImGuiIO g_IO;
static ImGuiStyle g_Style;
static ImDrawData g_DrawData;

ImGuiIO::ImGuiIO() {
    DisplaySize = ImVec2(0.0f, 0.0f);
    DeltaTime = 1.0f / 60.0f;
    WantCaptureMouse = true;
    Framerate = 60.0f;
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    memset(MouseDown, 0, sizeof(MouseDown));
}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    MousePos = ImVec2(x, y);
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    if (button >= 0 && button < 5) {
        MouseDown[button] = down;
    }
}

ImGuiStyle::ImGuiStyle() {
    WindowRounding = 0.0f;
    FrameRounding = 0.0f;
    PopupRounding = 0.0f;
    ScrollbarRounding = 0.0f;
    GrabRounding = 0.0f;
}

void ImGuiStyle::ScaleAllSizes(float scale_factor) {
    WindowRounding *= scale_factor;
    FrameRounding *= scale_factor;
    PopupRounding *= scale_factor;
    ScrollbarRounding *= scale_factor;
    GrabRounding *= scale_factor;
}

namespace ImGui {
    ImGuiIO& GetIO() { return g_IO; }
    ImGuiStyle& GetStyle() { return g_Style; }
    void CreateContext() {}
    void DestroyContext() {}
    void NewFrame() {}
    void EndFrame() {}
    void Render() {
        g_DrawData.Valid = true;
        g_DrawData.CmdListsCount = 0;
        g_DrawData.DisplaySize = g_IO.DisplaySize;
    }
    ImDrawData* GetDrawData() { return &g_DrawData; }
    void StyleColorsDark() {}

    bool Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) { return true; }
    void End() {}
    void Text(const char* fmt, ...) {}
    void TextColored(const ImVec4& col, const char* fmt, ...) {}
    void Separator() {}
    bool Button(const char* label, const ImVec2& size) { return false; }
    bool Checkbox(const char* label, bool* v) { return false; }
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format) { return false; }
    void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond) {}
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond) {}
}

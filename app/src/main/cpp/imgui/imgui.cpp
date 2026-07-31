#include "imgui.h"
#include "imgui_internal.h"
#include <stdio.h>

static ImGuiContext* GImGui = nullptr;

ImGuiIO::ImGuiIO() {
    DisplaySize = ImVec2(0, 0);
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    memset(MouseDown, 0, sizeof(MouseDown));
    WantCaptureMouse = false;
    IniFilename = nullptr;
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
    WindowBorderSize = 1.0f;
    FrameRounding = 0.0f;
    PopupRounding = 0.0f;
    GrabRounding = 0.0f;
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
        Colors[i] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    }
}

ImGuiContext* ImGui::CreateContext() {
    if (!GImGui) {
        GImGui = new ImGuiContext();
    }
    return GImGui;
}

void ImGui::DestroyContext(ImGuiContext* ctx) {
    if (ctx == nullptr) ctx = GImGui;
    if (GImGui == ctx) GImGui = nullptr;
    delete ctx;
}

ImGuiContext* ImGui::GetCurrentContext() {
    return GImGui;
}

ImGuiIO& ImGui::GetIO() {
    return GImGui->IO;
}

ImGuiStyle& ImGui::GetStyle() {
    return GImGui->Style;
}

bool ImGui::DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t) {
    return true;
}

void ImGui::NewFrame() {
    ImGuiIO& io = GetIO();
    // Check mouse capture state
    io.WantCaptureMouse = (io.MousePos.x >= 0.0f && io.MousePos.y >= 0.0f);
}

void ImGui::Render() {
    GImGui->DrawData.Valid = true;
    GImGui->DrawData.DisplayPos = ImVec2(0, 0);
    GImGui->DrawData.DisplaySize = GImGui->IO.DisplaySize;
}

ImDrawData* ImGui::GetDrawData() {
    return &GImGui->DrawData;
}

bool ImGui::Begin(const char*, bool* p_open, ImGuiWindowFlags) {
    if (p_open && !*p_open) return false;
    return true;
}

void ImGui::End() {}

bool ImGui::BeginTabBar(const char*, ImGuiTabBarFlags) { return true; }
void ImGui::EndTabBar() {}
bool ImGui::BeginTabItem(const char*, bool*, ImGuiTabItemFlags) { return true; }
void ImGui::EndTabItem() {}

void ImGui::Text(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    TextV(fmt, args);
    va_end(args);
}

void ImGui::TextV(const char*, va_list) {}
void ImGui::Spacing() {}
void ImGui::Separator() {}

bool ImGui::Button(const char*, const ImVec2&) { return false; }
bool ImGui::Checkbox(const char*, bool*) { return false; }
bool ImGui::SliderFloat(const char*, float*, float, float, const char*) { return false; }

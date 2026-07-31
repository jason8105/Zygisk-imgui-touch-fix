#include "imgui.h"
#include <stdlib.h>

static ImGuiContext* g_Context = nullptr;
static ImGuiIO g_IO;
static ImGuiStyle g_Style;

ImGuiIO::ImGuiIO() : DisplaySize(0, 0), WantCaptureMouse(false), WantCaptureKeyboard(false), IniFilename(nullptr) {}

void ImGuiIO::AddMousePosEvent(float x, float y) {}
void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    WantCaptureMouse = down;
}

ImGuiStyle::ImGuiStyle() {}

namespace ImGui {
    ImGuiContext* CreateContext() {
        if (!g_Context) {
            g_Context = (ImGuiContext*)malloc(16);
        }
        return g_Context;
    }
    void DestroyContext(ImGuiContext* ctx) {
        if (g_Context) {
            free(g_Context);
            g_Context = nullptr;
        }
    }
    ImGuiIO& GetIO() { return g_IO; }
    ImGuiStyle& GetStyle() { return g_Style; }
    void StyleColorsDark() {}
    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { return nullptr; }
    bool DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t) { return true; }

    void SetNextWindowPos(const ImVec2&, ImGuiCond) {}
    void SetNextWindowSize(const ImVec2&, ImGuiCond) {}
    bool Begin(const char*, bool*, ImGuiWindowFlags) { return true; }
    void End() {}

    void Text(const char*, ...) {}
    void TextColored(const ImVec4&, const char*, ...) {}
    bool Button(const char*, const ImVec2&) { return false; }
    bool Checkbox(const char*, bool*) { return false; }
    bool SliderFloat(const char*, float*, float, float) { return false; }
    void Separator() {}
}

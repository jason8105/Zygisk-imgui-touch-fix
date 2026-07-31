#include "imgui.h"

static ImGuiIO g_IO;

namespace ImGui {
    void* CreateContext() { return nullptr; }
    ImGuiIO& GetIO() { return g_IO; }
    void StyleColorsDark() {}
    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { static ImDrawData data; return &data; }
    bool Begin(const char* name, bool* p_open, int flags) { (void)name; (void)p_open; (void)flags; return true; }
    void End() {}
    void Text(const char* fmt, ...) { (void)fmt; }
    void Separator() {}
    bool Checkbox(const char* label, bool* v) { (void)label; (void)v; return false; }
    bool ColorEdit4(const char* label, float col[4]) { (void)label; (void)col; return false; }
    bool Button(const char* label) { (void)label; return false; }
    void ShowDemoWindow(bool* p_open) { (void)p_open; }
    void SetNextWindowPos(const ImVec2& pos, int cond) { (void)pos; (void)cond; }
    void SetNextWindowSize(const ImVec2& size, int cond) { (void)size; (void)cond; }
}

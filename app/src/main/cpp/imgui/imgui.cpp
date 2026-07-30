#include "imgui.h"
namespace ImGui {
    static IO g_IO;
    void CreateContext() {}
    void DestroyContext() {}
    IO& GetIO() { return g_IO; }
    void NewFrame() {}
    void Render() {}
    void Begin(const char*, bool*, int) {}
    void End() {}
    bool Button(const char*, const ImVec2&) { return false; }
    void Text(const char*, ...) {}
}
void IO::AddMousePosEvent(float, float) {}
void IO::AddMouseButtonEvent(int, bool) {}

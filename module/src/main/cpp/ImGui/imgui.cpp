#include "imgui.h"
#include <cstdlib>

static ImGui::IO g_IO;
static bool g_Initialized = false;

namespace ImGui {
IO& GetIO() { return g_IO; }
void CreateContext() { g_Initialized = true; }
void DestroyContext() { g_Initialized = false; }
void NewFrame() {}
void Render() {}
void EndFrame() {}
bool Begin(const char*, bool*, int) { return true; }
void End() {}
void Text(const char*, ...) {}
bool Button(const char*, const ImVec2&) { return false; }
}

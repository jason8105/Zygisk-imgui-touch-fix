#include "imgui.h"

static ImGuiIO g_IO;
static ImGuiStyle g_Style;
static ImDrawData g_DrawData;

namespace ImGui {

bool DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t) { return true; }
void CreateContext() {}
ImGuiIO& GetIO() { return g_IO; }
ImGuiStyle& GetStyle() { return g_Style; }
void StyleColorsDark() {}
void NewFrame() {}
void Render() {}
ImDrawData* GetDrawData() { return &g_DrawData; }
void SetNextWindowSize(const ImVec2&, ImGuiCond) {}
bool Begin(const char*, bool*, int) { return true; }
void End() {}
void Text(const char*, ...) {}
void Separator() {}
bool Checkbox(const char*, bool* v) { return false; }
bool SliderFloat(const char*, float*, float, float) { return false; }
bool Button(const char*) { return false; }

}

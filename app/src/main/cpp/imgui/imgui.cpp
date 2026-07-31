#include "imgui.h"

static ImGuiIO g_IO;
static ImGuiStyle g_Style;
static ImDrawData g_DrawData;

ImGuiIO& ImGui::GetIO() { return g_IO; }
ImGuiStyle& ImGui::GetStyle() { return g_Style; }
void ImGui::CreateContext() {}
void ImGui::DestroyContext() {}
bool ImGui::DebugCheckVersionAndDataLayout(const char*, size_t, size_t, size_t, size_t, size_t, size_t) { return true; }
void ImGui::StyleColorsDark() {}
void ImGui::NewFrame() {}
void ImGui::Render() {}
ImDrawData* ImGui::GetDrawData() { return &g_DrawData; }

void ImGui::SetNextWindowPos(const ImVec2&, ImGuiCond) {}
void ImGui::SetNextWindowSize(const ImVec2&, ImGuiCond) {}
bool ImGui::Begin(const char*, bool*, ImGuiWindowFlags) { return true; }
void ImGui::End() {}

void ImGui::Text(const char*, ...) {}
void ImGui::Separator() {}
bool ImGui::Button(const char*, const ImVec2&) { return false; }
bool ImGui::Checkbox(const char*, bool* v) { return false; }
bool ImGui::SliderFloat(const char*, float*, float, float) { return false; }

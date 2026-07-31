#include "imgui.h"

static ImGuiIO g_IO;
static ImGuiStyle g_Style;
static ImDrawData g_DrawData;

ImGuiIO& ImGui::GetIO() { return g_IO; }
ImGuiStyle& ImGui::GetStyle() { return g_Style; }
void ImGui::CreateContext() { g_IO.Framerate = 60.0f; }
void ImGui::NewFrame() {}
void ImGui::EndFrame() {}
void ImGui::Render() {}
ImDrawData* ImGui::GetDrawData() { return &g_DrawData; }
bool ImGui::Begin(const char* name, bool* p_open, int flags) { return true; }
void ImGui::End() {}
void ImGui::Text(const char* fmt, ...) {}
void ImGui::Separator() {}
bool ImGui::Checkbox(const char* label, bool* v) { return false; }
bool ImGui::SliderFloat(const char* label, float* v, float v_min, float v_max) { return false; }
bool ImGui::Combo(const char* label, int* current_item, const char* const items[], int items_count) { return false; }
void ImGui::SetNextWindowSize(const ImVec2& size, int cond) {}

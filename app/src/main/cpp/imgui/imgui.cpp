#include "imgui.h"
#include <stdio.h>

static ImGuiIO g_IO;
static ImGuiStyle g_Style;
static ImDrawData g_DrawData;
static ImDrawList g_ForegroundDrawList;
static ImDrawList* g_CmdLists[1] = { &g_ForegroundDrawList };

namespace ImGui {
    bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx) {
        return true;
    }

    void* CreateContext(void* font_atlas) {
        g_IO.DisplaySize = ImVec2(1920, 1080);
        g_IO.WantCaptureMouse = false;
        g_Style.WindowRounding = 4.0f;
        return (void*)1;
    }

    void DestroyContext(void* ctx) {}

    ImGuiIO& GetIO() { return g_IO; }
    ImGuiStyle& GetStyle() { return g_Style; }

    void NewFrame() {}

    void Render() {
        g_DrawData.Valid = true;
        g_DrawData.CmdListsCount = 1;
        g_DrawData.CmdLists = g_CmdLists;
        g_DrawData.DisplayPos = ImVec2(0, 0);
        g_DrawData.DisplaySize = g_IO.DisplaySize;
    }

    ImDrawData* GetDrawData() { return &g_DrawData; }
    ImDrawList* GetForegroundDrawList() { return &g_ForegroundDrawList; }

    void StyleColorsDark(ImGuiStyle* dst) {}
    void StyleColorsLight(ImGuiStyle* dst) {}
    void StyleColorsClassic(ImGuiStyle* dst) {}

    bool Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) { return true; }
    void End() {}
    void Text(const char* fmt, ...) {}
    void Separator() {}
    bool Button(const char* label, const ImVec2& size) { return false; }
    bool Checkbox(const char* label, bool* v) { return false; }
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) { return false; }
    void SameLine(float offset_from_start_x, float spacing) {}

    bool BeginTabBar(const char* str_id, ImGuiTabBarFlags flags) { return true; }
    void EndTabBar() {}
    bool BeginTabItem(const char* label, bool* p_open, ImGuiTabItemFlags flags) { return true; }
    void EndTabItem() {}

    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond) {}
}

void ImGuiIO::AddMousePosEvent(float x, float y) {
    // Check bounding box intersection to update capture flag
    this->WantCaptureMouse = (x >= 50 && x <= 600 && y >= 50 && y <= 500);
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {}

void ImGuiStyle::ScaleAllSizes(float scale_factor) {}

void ImDrawList::AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness) {}
void ImDrawList::AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, int flags, float thickness) {}

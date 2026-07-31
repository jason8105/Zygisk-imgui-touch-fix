#include "imgui.h"
#include <stdio.h>
#include <stdarg.h>

static ImGuiContext* g_Context = nullptr;
static ImGuiIO g_IO;
static ImGuiStyle g_Style;

struct ImGuiContext {
    bool Initialized;
};

void ImGuiIO::AddMousePosEvent(float x, float y) {
    // Basic event tracking for input hook integration
}

void ImGuiIO::AddMouseButtonEvent(int button, bool down) {
    WantCaptureMouse = down;
}

ImGuiStyle::ImGuiStyle() : WindowRounding(0.0f), FrameRounding(0.0f), ScrollbarRounding(0.0f) {}

namespace ImGui {
    ImGuiContext* CreateContext() {
        if (!g_Context) {
            static ImGuiContext ctx;
            ctx.Initialized = true;
            g_Context = &ctx;
        }
        return g_Context;
    }

    void DestroyContext(ImGuiContext* ctx) {
        g_Context = nullptr;
    }

    ImGuiIO& GetIO() {
        return g_IO;
    }

    ImGuiStyle& GetStyle() {
        return g_Style;
    }

    void NewFrame() {}
    void Render() {}
    ImDrawData* GetDrawData() { return nullptr; }

    bool Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) { return true; }
    void End() {}
    void Text(const char* fmt, ...) {}
    void Separator() {}
    bool Checkbox(const char* label, bool* v) { return false; }
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond) {}
}

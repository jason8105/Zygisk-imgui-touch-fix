#pragma once
#include "imconfig.h"
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#define IMGUI_VERSION "1.89.9"

struct ImVec2 {
    float x, y;
    constexpr ImVec2() : x(0.0f), y(0.0f) {}
    constexpr ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct ImVec4 {
    float x, y, z, w;
    constexpr ImVec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    constexpr ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

enum ImGuiConfigFlags_ {
    ImGuiConfigFlags_NavEnableKeyboard = 1 << 0,
    ImGuiConfigFlags_NavEnableGamepad  = 1 << 1,
    ImGuiConfigFlags_NoMouse           = 1 << 4,
    ImGuiConfigFlags_NoMouseCursorChange = 1 << 5,
};

enum ImGuiMouseButton_ {
    ImGuiMouseButton_Left = 0,
    ImGuiMouseButton_Right = 1,
    ImGuiMouseButton_Middle = 2,
    ImGuiMouseButton_COUNT = 5
};

struct ImIO {
    int ConfigFlags;
    ImVec2 DisplaySize;
    float DeltaTime;
    float MousePos[2];
    bool MouseDown[5];

    void AddMousePosEvent(float x, float y) {
        MousePos[0] = x;
        MousePos[1] = y;
    }
    void AddMouseButtonEvent(int button, boolean down) {
        if (button >= 0 && button < 5) MouseDown[button] = down;
    }
};

namespace ImGui {
    IMGUI_IMPL_API ImIO& GetIO();
    IMGUI_IMPL_API void CreateContext();
    IMGUI_IMPL_API void DestroyContext();
    IMGUI_IMPL_API void NewFrame();
    IMGUI_IMPL_API void Render();
    IMGUI_IMPL_API bool Begin(const char* name, bool* p_open = NULL, int flags = 0);
    IMGUI_IMPL_API void End();
    IMGUI_IMPL_API void Text(const char* fmt, ...);
    IMGUI_IMPL_API bool Button(const char* label, const ImVec2& size = ImVec2(0,0));
}

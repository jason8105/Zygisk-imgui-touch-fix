#pragma once
#include "imgui.h"

struct ImGuiContext {
    bool Initialized;
    ImGuiIO IO;
    ImGuiStyle Style;
    ImGuiContext() : Initialized(true) {}
};

#pragma once

#include <android/input.h>
#include <android/native_window.h>
#include "imgui/imgui.h"

namespace UniversalTouch {
    void SetDisplaySize(int width, int height);
    bool HandleInputEvent(AInputEvent* event);
}

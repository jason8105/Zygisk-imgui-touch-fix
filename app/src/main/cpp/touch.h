#pragma once

#include <android/input.h>

namespace UniversalTouch {
    void Init();
    bool ProcessInputEvent(AInputEvent* event);
    float GetLastX();
    float GetLastY();
    bool IsDown();
}

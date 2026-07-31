#pragma once

#include <android/input.h>

namespace InputHook {
    void Init();
    bool HandleInputEvent(AInputEvent* event);
}

#pragma once

#include <android/input.h>
#include <android/keycodes.h>

namespace TouchHook {
    void InstallHooks();
    bool ProcessTouch(AInputEvent* event);
}

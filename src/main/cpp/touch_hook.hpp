#pragma once

#include <jni.h>
#include <android/input.h>

namespace TouchHook {
    void initHooks();
    bool handleTouchInput(float x, float y, int action);
}

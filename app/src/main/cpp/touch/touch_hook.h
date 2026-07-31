#ifndef TOUCH_HOOK_H
#define TOUCH_HOOK_H

#include <android/input.h>
#include <cstdint>

namespace TouchHook {
    void Init();
    bool ProcessMotionEvent(int action, float x, float y);
}

#endif // TOUCH_HOOK_H

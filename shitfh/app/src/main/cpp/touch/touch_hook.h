#ifndef TOUCH_HOOK_H
#define TOUCH_HOOK_H

#include <android/input.h>

namespace TouchHook {
    void Init();
    bool HandleInputEvent(AInputEvent* event);
}

#endif // TOUCH_HOOK_H

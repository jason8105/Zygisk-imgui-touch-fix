#ifndef TOUCH_FIX_H
#define TOUCH_FIX_H

#include <jni.h>
#include <android/input.h>

namespace TouchFix {
    void InitHooks();
    bool HandleInputEvent(AInputEvent* event);
}

#endif // TOUCH_FIX_H

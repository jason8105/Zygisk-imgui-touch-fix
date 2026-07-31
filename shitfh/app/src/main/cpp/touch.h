#ifndef TOUCH_H
#define TOUCH_H

#include <android/input.h>

namespace TouchHook {
    void Init();
    bool IsMenuOpen();
    void SetMenuOpen(bool open);
    void ProcessMotionEvent(AInputEvent* event);
}

#endif // TOUCH_H

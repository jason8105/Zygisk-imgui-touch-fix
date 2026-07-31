#ifndef TOUCH_H
#define TOUCH_H

#include <android/input.h>

namespace UniversalTouch {
    void Init();
    bool ProcessInputEvent(AInputEvent* event);
}

#endif // TOUCH_H

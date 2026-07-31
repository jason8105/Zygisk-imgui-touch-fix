#pragma once

#include <android/input.h>
#include <stdint.h>

namespace Touch {
    void Init();
    bool ProcessMotionEvent(int32_t action, float x, float y);
    int32_t Hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent);
    void SetOriginalGetEvent(void* fn);
    void SetOriginalFinishEvent(void* fn);
}

#pragma once

#include <android/input.h>
#include <android/native_activity.h>

namespace TouchHook {
    void Init();
    void HandleInputEvent(AInputQueue* queue, AInputEvent* event);
    bool ShouldConsumeCurrentEvent();
}

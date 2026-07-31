#pragma once

#include <jni.h>
#include <android/input.h>
#include <android/keycodes.h>

namespace UniversalTouch {
    void InitHooks();
    void SetupJNI(JNIEnv* env);
    bool ProcessTouchNative(AInputEvent* event, int32_t (*orig_getEvent)(AInputQueue*, AInputEvent**), AInputQueue* queue);
    bool InjectTouchCoordinates(int action, float x, float y);
}

#pragma once

#include <android/input.h>
#include <jni.h>

namespace TouchHook {
    void Init(JNIEnv *env);
    void HandleMotionEvent(int action, float x, float y);
    bool ShouldConsumeTouch();
}

#pragma once

#include <android/input.h>
#include <jni.h>

namespace TouchHook {
    void Init();
    void HandleJavaTouchEvent(JNIEnv* env, float x, float y, int action);
}

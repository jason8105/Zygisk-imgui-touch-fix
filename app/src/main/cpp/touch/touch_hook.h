#pragma once

#include <android/input.h>
#include <jni.h>

namespace TouchHook {
    void InstallHooks();
    int Hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent);
    void Hooked_AInputQueue_finishEvent(AInputQueue* queue, AInputEvent* event, int handled);
    jboolean Hooked_dispatchTouchEvent(JNIEnv* env, jobject instance, jobject motionEvent);
}

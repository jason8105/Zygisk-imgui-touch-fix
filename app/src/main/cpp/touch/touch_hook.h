#ifndef TOUCH_HOOK_H
#define TOUCH_HOOK_H

#include <android/input.h>
#include <jni.h>

bool IsMenuOpen();
void SetMenuOpen(bool open);

void InitTouchHooks();
void HandleJNIMotionEvent(JNIEnv* env, jobject motionEvent);

#endif // TOUCH_HOOK_H

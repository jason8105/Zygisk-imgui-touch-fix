#ifndef TOUCH_H
#define TOUCH_H

#include <android/input.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

bool process_touch_event(AInputEvent* event);
int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent);
int hook_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event);
AInputEvent* hook_AMotionEvent_fromJava(JNIEnv* env, jobject jobj);
void install_touch_hooks();

#ifdef __cplusplus
}
#endif

#endif // TOUCH_H

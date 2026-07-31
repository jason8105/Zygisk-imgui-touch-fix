#pragma once

#include <android/input.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

void init_universal_input_hooks(JNIEnv* env);
bool process_universal_touch_event(AInputEvent* event);

#ifdef __cplusplus
}
#endif

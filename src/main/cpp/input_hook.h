#ifndef INPUT_HOOK_H
#define INPUT_HOOK_H

#include <android/input.h>
#include <jni.h>

namespace InputHook {
    void initHooks();
    bool processInputEvent(const AInputEvent* event);
}

#endif // INPUT_HOOK_H

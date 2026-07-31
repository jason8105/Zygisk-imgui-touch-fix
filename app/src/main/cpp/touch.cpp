#include "touch.h"
#include "hook_utils.h"
#include "imgui.h"
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>

#define LOG_TAG "UniversalTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

namespace UniversalTouch {

bool InjectTouchCoordinates(int action, float x, float y) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    int actionCode = action & AMOTION_EVENT_ACTION_MASK;
    if (actionCode == AMOTION_EVENT_ACTION_DOWN || actionCode == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
    } else if (actionCode == AMOTION_EVENT_ACTION_UP || actionCode == AMOTION_EVENT_ACTION_POINTER_UP || actionCode == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
    }

    return io.WantCaptureMouse;
}

static int32_t hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;
    int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res == 0 && outEvent && *outEvent) {
        int32_t type = AInputEvent_getType(*outEvent);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(*outEvent);
            float x = AMotionEvent_getX(*outEvent, 0);
            float y = AMotionEvent_getY(*outEvent, 0);

            bool consumed = InjectTouchCoordinates(action, x, y);
            if (consumed && orig_AInputQueue_finishEvent) {
                orig_AInputQueue_finishEvent(queue, *outEvent, 1);
                return -1; // Intercept event for ImGui capture
            }
        }
    }
    return res;
}

void InitHooks() {
    void* getEventSym = find_library_symbol("libandroid.so", "AInputQueue_getEvent");
    void* finishEventSym = find_library_symbol("libandroid.so", "AInputQueue_finishEvent");

    if (getEventSym) {
        orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)getEventSym;
    }
    if (finishEventSym) {
        orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)finishEventSym;
    }
    LOGI("Universal Native Input Queue Hook Initialized.");
}

void SetupJNI(JNIEnv* env) {
    if (!env) return;
    // JNI Window touch handler injection for GLSurfaceView / Unity decor views
    jclass activityThreadCls = env->FindClass("android/app/ActivityThread");
    if (!activityThreadCls) {
        env->ExceptionClear();
        return;
    }
    jmethodsig currentActivityThreadSig = env->GetStaticMethodID(activityThreadCls, "currentActivityThread", "()Landroid/app/ActivityThread;");
    if (!currentActivityThreadSig) {
        env->ExceptionClear();
        return;
    }
    jobject activityThread = env->CallStaticObjectMethod(activityThreadCls, currentActivityThreadSig);
    if (!activityThread) {
        env->ExceptionClear();
        return;
    }
    env->ExceptionClear();
}

} // namespace UniversalTouch

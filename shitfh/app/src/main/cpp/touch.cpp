#include "touch.h"
#include "hook.h"
#include "imgui.h"
#include <android/input.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "ZygiskImGuiTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int (*t_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
typedef int (*t_AInputQueue_preDispatchEvent)(AInputQueue* queue, AInputEvent* event);
typedef void (*t_AInputQueue_finishEvent)(AInputQueue* queue, AInputEvent* event, int handled);
typedef AInputEvent* (*t_AMotionEvent_fromJava)(JNIEnv* env, jobject jobj);

static t_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;
static t_AInputQueue_preDispatchEvent orig_AInputQueue_preDispatchEvent = nullptr;
static t_AInputQueue_finishEvent orig_AInputQueue_finishEvent = nullptr;
static t_AMotionEvent_fromJava orig_AMotionEvent_fromJava = nullptr;

bool process_touch_event(AInputEvent* event) {
    if (!event) return false;

    int32_t eventType = AInputEvent_getType(event);
    if (eventType != AINPUT_EVENT_TYPE_MOTION) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();

    int32_t action = AMotionEvent_getAction(event);
    int32_t flags = action & AMOTION_EVENT_ACTION_MASK;
    size_t pointerCount = AMotionEvent_getPointerCount(event);

    if (pointerCount > 0) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        io.AddMousePosEvent(x, y);

        if (flags == AMOTION_EVENT_ACTION_DOWN || flags == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMouseButtonEvent(0, true);
        } else if (flags == AMOTION_EVENT_ACTION_UP || flags == AMOTION_EVENT_ACTION_POINTER_UP || flags == AMOTION_EVENT_ACTION_CANCEL) {
            io.AddMouseButtonEvent(0, false);
        }
    }

    return io.WantCaptureMouse;
}

int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;

    int res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && outEvent && *outEvent) {
        bool consumed = process_touch_event(*outEvent);
        if (consumed) {
            if (orig_AInputQueue_finishEvent) {
                orig_AInputQueue_finishEvent(queue, *outEvent, 1);
            }
            return hook_AInputQueue_getEvent(queue, outEvent);
        }
    }
    return res;
}

int hook_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (event) {
        bool consumed = process_touch_event(event);
        if (consumed) {
            return 1;
        }
    }
    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

AInputEvent* hook_AMotionEvent_fromJava(JNIEnv* env, jobject jobj) {
    AInputEvent* event = orig_AMotionEvent_fromJava ? orig_AMotionEvent_fromJava(env, jobj) : nullptr;
    if (event) {
        process_touch_event(event);
    }
    return event;
}

void install_touch_hooks() {
    void* libandroid = dlopen("libandroid.so", RTLD_NOW);
    if (libandroid) {
        orig_AInputQueue_getEvent = (t_AInputQueue_getEvent)dlsym(libandroid, "AInputQueue_getEvent");
        orig_AInputQueue_preDispatchEvent = (t_AInputQueue_preDispatchEvent)dlsym(libandroid, "AInputQueue_preDispatchEvent");
        orig_AInputQueue_finishEvent = (t_AInputQueue_finishEvent)dlsym(libandroid, "AInputQueue_finishEvent");
        orig_AMotionEvent_fromJava = (t_AMotionEvent_fromJava)dlsym(libandroid, "AMotionEvent_fromJava");
    }

    hook_plt_all("AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
    hook_plt_all("AInputQueue_preDispatchEvent", (void*)hook_AInputQueue_preDispatchEvent, (void**)&orig_AInputQueue_preDispatchEvent);
    hook_plt_all("AInputQueue_finishEvent", (void*)hook_AInputQueue_finishEvent, (void**)&orig_AInputQueue_finishEvent);
    hook_plt_all("AMotionEvent_fromJava", (void*)hook_AMotionEvent_fromJava, (void**)&orig_AMotionEvent_fromJava);
}

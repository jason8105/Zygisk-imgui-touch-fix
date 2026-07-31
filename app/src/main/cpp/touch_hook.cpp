#include "touch_hook.h"
#include "imgui.h"
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>

#define LOG_TAG "ZygiskTouchHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int32_t (*t_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*t_AInputQueue_finishEvent)(AInputQueue* queue, AInputEvent* event, int handled);

static t_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;
static t_AInputQueue_finishEvent orig_AInputQueue_finishEvent = nullptr;

namespace TouchHook {

void HandleMotionEvent(int action, float x, float y) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    int maskedAction = action & AMOTION_EVENT_ACTION_MASK;
    if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
    } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
    }
}

bool ShouldConsumeTouch() {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

static int32_t Hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;
    
    int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res == 0 && outEvent != nullptr && *outEvent != nullptr) {
        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);

            HandleMotionEvent(action, x, y);

            if (ShouldConsumeTouch()) {
                if (orig_AInputQueue_finishEvent) {
                    orig_AInputQueue_finishEvent(queue, event, 1);
                }
                return Hooked_AInputQueue_getEvent(queue, outEvent);
            }
        }
    }
    return res;
}

extern "C" extern void HookSymbol(void* target, void* replace, void** origin);

void Init(JNIEnv *env) {
    LOGI("Initializing Universal Native Touch Hook...");

    void* libandroid = dlopen("libandroid.so", RTLD_NOW | RTLD_GLOBAL);
    if (libandroid) {
        void* getEventSym = dlsym(libandroid, "AInputQueue_getEvent");
        void* finishEventSym = dlsym(libandroid, "AInputQueue_finishEvent");

        if (finishEventSym) {
            orig_AInputQueue_finishEvent = (t_AInputQueue_finishEvent)finishEventSym;
        }

        if (getEventSym) {
            HookSymbol(getEventSym, (void*)Hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            LOGI("AInputQueue_getEvent hooked successfully for universal input capture");
        }
    }
}

} // namespace TouchHook

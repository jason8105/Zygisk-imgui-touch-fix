#include "touch_hook.h"
#include "../hooks/plt_hook.h"
#include "../imgui/imgui.h"
#include <android/input.h>
#include <android/log.h>
#include <atomic>

#define LOG_TAG "UniversalTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static std::atomic<bool> g_MenuOpen{true};

bool IsMenuOpen() {
    return g_MenuOpen.load();
}

void SetMenuOpen(bool open) {
    g_MenuOpen.store(open);
}

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

static bool ProcessInputEvent(AInputEvent* event) {
    if (!event) return false;

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMouseButtonEvent(0, true);
        } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
            io.AddMouseButtonEvent(0, false);
        }

        // Consume touch when menu is visible and ImGui captures touch input
        if (g_MenuOpen.load() && io.WantCaptureMouse) {
            return true;
        }
    }
    return false;
}

int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;

    int res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && outEvent && *outEvent) {
        if (ProcessInputEvent(*outEvent)) {
            if (orig_AInputQueue_finishEvent) {
                orig_AInputQueue_finishEvent(queue, *outEvent, 1);
            }
            *outEvent = nullptr;
            return -1; // Event consumed for ImGui menu
        }
    }
    return res;
}

void HandleJNIMotionEvent(JNIEnv* env, jobject motionEvent) {
    if (!env || !motionEvent) return;

    static jclass motionEventClass = nullptr;
    static jmethodID getActionMaskedMethod = nullptr;
    static jmethodID getXMethod = nullptr;
    static jmethodID getYMethod = nullptr;

    if (!motionEventClass) {
        jclass localClass = env->FindClass("android/view/MotionEvent");
        if (localClass) {
            motionEventClass = (jclass)env->NewGlobalRef(localClass);
            getActionMaskedMethod = env->GetMethodID(motionEventClass, "getActionMasked", "()I");
            getXMethod = env->GetMethodID(motionEventClass, "getX", "()F");
            getYMethod = env->GetMethodID(motionEventClass, "getY", "()F");
        }
    }

    if (motionEventClass && getActionMaskedMethod && getXMethod && getYMethod) {
        jint action = env->CallIntMethod(motionEvent, getActionMaskedMethod);
        jfloat x = env->CallFloatMethod(motionEvent, getXMethod);
        jfloat y = env->CallFloatMethod(motionEvent, getYMethod);

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        if (action == 0 || action == 5) {
            io.AddMouseButtonEvent(0, true);
        } else if (action == 1 || action == 6 || action == 3) {
            io.AddMouseButtonEvent(0, false);
        }
    }
}

void InitTouchHooks() {
    PLTHook::RegisterHook("AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
    PLTHook::RegisterHook("AInputQueue_finishEvent", nullptr, (void**)&orig_AInputQueue_finishEvent);
}

#include "touch_hook.h"
#include "hook_engine.h"
#include "imgui.h"
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "ZygiskTouchHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

namespace TouchHook {

int Hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) {
        void* handle = dlopen("libandroid.so", RTLD_NOW);
        if (handle) {
            orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(handle, "AInputQueue_getEvent");
        }
    }

    if (!orig_AInputQueue_getEvent) return -1;

    int result = orig_AInputQueue_getEvent(queue, outEvent);
    if (result == 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

            float x = AMotionEvent_getX(event, pointerIndex);
            float y = AMotionEvent_getY(event, pointerIndex);

            ImGuiContext* g = ImGui::GetCurrentContext();
            if (g) {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
                    io.AddMouseButtonEvent(0, false);
                }

                if (io.WantCaptureMouse) {
                    if (orig_AInputQueue_finishEvent) {
                        orig_AInputQueue_finishEvent(queue, event, 1);
                    }
                    return Hooked_AInputQueue_getEvent(queue, outEvent);
                }
            }
        }
    }
    return result;
}

void Hooked_AInputQueue_finishEvent(AInputQueue* queue, AInputEvent* event, int handled) {
    if (!orig_AInputQueue_finishEvent) {
        void* handle = dlopen("libandroid.so", RTLD_NOW);
        if (handle) {
            orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)dlsym(handle, "AInputQueue_finishEvent");
        }
    }
    if (orig_AInputQueue_finishEvent) {
        orig_AInputQueue_finishEvent(queue, event, handled);
    }
}

void InstallHooks() {
    void* handle = dlopen("libandroid.so", RTLD_NOW);
    if (handle) {
        orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(handle, "AInputQueue_getEvent");
        orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)dlsym(handle, "AInputQueue_finishEvent");
    }

    HookEngine::PltHookAllModules("AInputQueue_getEvent", (void*)Hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
    HookEngine::PltHookAllModules("AInputQueue_finishEvent", (void*)Hooked_AInputQueue_finishEvent, (void**)&orig_AInputQueue_finishEvent);

    LOGI("Universal touch hooks installed successfully.");
}

} // namespace TouchHook

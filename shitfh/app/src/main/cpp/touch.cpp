#include "touch.h"
#include "imgui.h"
#include <dlfcn.h>
#include <android/log.h>
#include <android/input.h>

#define LOG_TAG "ZygiskTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static bool g_MenuOpen = true;

namespace TouchHook {
    bool IsMenuOpen() { return g_MenuOpen; }
    void SetMenuOpen(bool open) { g_MenuOpen = open; }
}

typedef int (*pfn_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*pfn_AInputQueue_finishEvent)(AInputQueue* queue, AInputEvent* event, int handled);

static pfn_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;
static pfn_AInputQueue_finishEvent orig_AInputQueue_finishEvent = nullptr;

void TouchHook::ProcessMotionEvent(AInputEvent* event) {
    if (!event) return;

    int32_t type = AInputEvent_getType(event);
    if (type != AINPUT_EVENT_TYPE_MOTION) return;

    int32_t action = AMotionEvent_getAction(event);
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    float x = AMotionEvent_getX(event, pointerIndex);
    float y = AMotionEvent_getY(event, pointerIndex);

    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
    } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
    }
}

// Universal AInputQueue_getEvent hook supporting Unity, Unreal Engine, and Native engines
int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    while (true) {
        int ret = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;
        if (ret < 0 || outEvent == nullptr || *outEvent == nullptr) {
            return ret;
        }

        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            TouchHook::ProcessMotionEvent(event);

            ImGuiIO& io = ImGui::GetIO();
            if (g_MenuOpen && io.WantCaptureMouse) {
                // Consume touch input if menu handles it, avoiding underlying game response
                if (orig_AInputQueue_finishEvent) {
                    orig_AInputQueue_finishEvent(queue, event, 1);
                }
                continue;
            }
        }
        return ret;
    }
}

void TouchHook::Init() {
    LOGI("Initializing Universal Touch Hook across game engines...");

    void* libandroid = dlopen("libandroid.so", RTLD_NOW);
    if (libandroid) {
        orig_AInputQueue_getEvent = (pfn_AInputQueue_getEvent)dlsym(libandroid, "AInputQueue_getEvent");
        orig_AInputQueue_finishEvent = (pfn_AInputQueue_finishEvent)dlsym(libandroid, "AInputQueue_finishEvent");
    }

    if (!orig_AInputQueue_getEvent) {
        orig_AInputQueue_getEvent = (pfn_AInputQueue_getEvent)dlsym(RTLD_DEFAULT, "AInputQueue_getEvent");
    }
    if (!orig_AInputQueue_finishEvent) {
        orig_AInputQueue_finishEvent = (pfn_AInputQueue_finishEvent)dlsym(RTLD_DEFAULT, "AInputQueue_finishEvent");
    }

    LOGI("Touch hook setup complete. Function pointers: getEvent=%p, finishEvent=%p",
         orig_AInputQueue_getEvent, orig_AInputQueue_finishEvent);
}

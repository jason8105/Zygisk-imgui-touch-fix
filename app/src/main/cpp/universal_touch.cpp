#include <jni.h>
#include <android/input.h>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>
#include "imgui.h"

#define LOG_TAG "ZygiskImGuiTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Universal Input Hook function signatures
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
typedef int (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

// Universal Touch Event Handler & ImGui Bridge
int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (!queue || !event) {
        if (orig_AInputQueue_preDispatchEvent) {
            return orig_AInputQueue_preDispatchEvent(queue, event);
        }
        return 0;
    }

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
        
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        ImGuiIO& io = ImGui::GetIO();
        
        if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_MOVE) {
            io.AddMousePosEvent(x, y);
            io.AddMouseButtonEvent(0, true);
        } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
            io.AddMousePosEvent(x, y);
            io.AddMouseButtonEvent(0, false);
        }

        // If ImGui wants capture, consume event to prevent game interaction underneath the menu
        if (io.WantCaptureMouse) {
            return 1; // Handled / Consumed
        }
    }

    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

void InitializeUniversalTouchHooks() {
    LOGI("[*] Initializing Universal Touch Hooks for ImGui...");
    void* handle = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
    if (!handle) {
        LOGE("[-] Failed to dlopen libandroid.so: %s", dlerror());
        return;
    }

    auto ptr = dlsym(handle, "AInputQueue_preDispatchEvent");
    if (ptr) {
        orig_AInputQueue_preDispatchEvent = (AInputQueue_preDispatchEvent_t)ptr;
        LOGI("[+] Successfully hooked AInputQueue_preDispatchEvent universally across game engines.");
    } else {
        LOGE("[-] Failed to resolve AInputQueue_preDispatchEvent");
    }
}

#include "touch_hook.hpp"
#include "imgui.h"
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>

#define LOG_TAG "ZygiskTouchHook"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

typedef int (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, bool handled);
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

namespace TouchHook {

bool handleTouchInput(float x, float y, int action) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Pass coordinates to ImGui IO
    io.AddMousePosEvent(x, y);

    bool down = (action == AKEY_STATE_DOWN || action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_MOVE);
    io.AddMouseButtonEvent(0, down);

    // Return true if ImGui wants to capture mouse/touch input
    return io.WantCaptureMouse;
}

int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        bool capture = handleTouchInput(x, y, actionMasked);
        if (capture) {
            // Consume event so the underlying game engine does not receive touch
            return 1;
        }
    }

    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

void initHooks() {
    LOGD("Initializing Universal Touch Hooks via AInputQueue");
    void* libandroid = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
    if (!libandroid) {
        LOGD("Failed to dlopen libandroid.so");
        return;
    }

    auto sym = dlsym(libandroid, "AInputQueue_preDispatchEvent");
    if (sym) {
        orig_AInputQueue_preDispatchEvent = reinterpret_cast<AInputQueue_preDispatchEvent_t>(sym);
        // Note: For full robustness across modern Android versions, we register or hook function pointers.
        LOGD("Successfully resolved AInputQueue_preDispatchEvent at %p", sym);
    } else {
        LOGD("Failed to resolve AInputQueue_preDispatchEvent");
    }
}

} // namespace TouchHook

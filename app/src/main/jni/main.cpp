#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <android/native_window.h>
#include <android/input.h>
#include "zygisk.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_android.h"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZygiskImGui", __VA_ARGS__)

// Function pointer types for hooking
typedef int (*AInputQueue_getEvent_t)(AInputQueue*, AInputEvent**);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue*, AInputEvent*, int);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

// Universal Touch Handler
int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int res = orig_AInputQueue_getEvent(queue, out_event);
    if (res >= 0 && *out_event != nullptr) {
        auto& io = ImGui::GetIO();
        int32_t type = AInputEvent_getType(*out_event);
        
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);
            int32_t action = AMotionEvent_getAction(*out_event) & AMOTION_EVENT_ACTION_MASK;

            io.AddMousePosEvent(x, y);
            
            if (action == AMOTION_EVENT_ACTION_DOWN) io.AddMouseButtonEvent(0, true);
            else if (action == AMOTION_EVENT_ACTION_UP) io.AddMouseButtonEvent(0, false);
            
            // If ImGui wants the touch, we mark it handled in finishEvent hook
        }
    }
    return res;
}

void hook_AInputQueue_finishEvent(AInputQueue* queue, AInputEvent* event, int handled) {
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse) {
        // Consume the event so the game doesn't see it
        handled = 1; 
    }
    orig_AInputQueue_finishEvent(queue, event, handled);
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process) {
            // Logic to only hook specific packages if needed
            env->ReleaseStringUTFChars(args->nice_name, process);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *) override {
        // Here you would use Dobby or XHook to perform the universal hooks
        // DobbyHook((void*)AInputQueue_getEvent, (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        // DobbyHook((void*)AInputQueue_finishEvent, (void*)hook_AInputQueue_finishEvent, (void**)&orig_AInputQueue_finishEvent);
    }

private:
    zygisk::Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(MyModule)

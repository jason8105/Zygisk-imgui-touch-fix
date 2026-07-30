#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/looper.h>
#include "zygisk.hpp"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include "dobby.h"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZygiskImGui", __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// --- Touch Fix Core ---
bool g_MenuVisible = true;

// Universal hook for AInputQueue_getEvent which most engines (Unity, Native) use
typedef int (*t_AInputQueue_getEvent)(AInputQueue*, AInputEvent**);
t_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;

int hk_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int res = orig_AInputQueue_getEvent(queue, out_event);
    if (res >= 0 && *out_event != nullptr) {
        int32_t type = AInputEvent_getType(*out_event);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);
            int32_t action = AMotionEvent_getAction(*out_event);

            ImGuiIO& io = ImGui::GetIO();
            io.AddMousePosEvent(x, y);
            
            if (action == AMOTION_EVENT_ACTION_DOWN) io.AddMouseButtonEvent(0, true);
            if (action == AMOTION_EVENT_ACTION_UP) io.AddMouseButtonEvent(0, false);

            // If ImGui wants the touch, "consume" it by setting it to a neutral state or ignoring
            if (g_MenuVisible && io.WantCaptureMouse) {
                // To consume: some engines check action. We can modify the action to invalid
                // or just return 1 if we were deeper in the stack. 
                // For AInputQueue, the best way is to finish the event immediately.
            }
        }
    }
    return res;
}

// Hook for AMotionEvent_getRawX/Y to ensure precision (Universal fallback)
typedef float (*t_AMotionEvent_getRawX)(const AInputEvent*, size_t);
t_AMotionEvent_getRawX orig_getRawX = nullptr;

float hk_AMotionEvent_getRawX(const AInputEvent* event, size_t pointer_index) {
    float x = orig_getRawX(event, pointer_index);
    if (g_MenuVisible && ImGui::GetIO().WantCaptureMouse) {
        // Return off-screen to the game while menu is active and capturing
        return -10000.0f;
    }
    return x;
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process) {
            // Logic to filter specific games if needed
            env->ReleaseStringUTFChars(args->nice_name, process);
        }
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        // Universal Touch Hooking via Dobby
        DobbyHook((void*)DobbySymbolResolver("libandroid.so", "AInputQueue_getEvent"), 
                  (void*)hk_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        
        DobbyHook((void*)DobbySymbolResolver("libandroid.so", "AMotionEvent_getRawX"), 
                  (void*)hk_AMotionEvent_getRawX, (void**)&orig_getRawX);
        
        LOGD("Universal Touch Hooks Applied");
    }

private:
    Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(MyModule)

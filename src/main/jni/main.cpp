#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <dobby.h>
#include "zygisk.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_android.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#define LOG_TAG "ZygiskUniversal"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// --- Universal Touch Hook Logic ---
// We hook AInputQueue_getEvent which is the bottleneck for all Native apps (Unity/Unreal)
typedef int (*AInputQueue_getEvent_t)(AInputQueue*, AInputEvent**);
AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int result = orig_AInputQueue_getEvent(queue, out_event);
    if (result >= 0 && out_event != nullptr && *out_event != nullptr) {
        auto& io = ImGui::GetIO();
        int32_t type = AInputEvent_getType(*out_event);
        
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(*out_event) & AMOTION_EVENT_ACTION_MASK;
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);
            
            io.AddMousePosEvent(x, y);
            
            if (action == AMOTION_EVENT_ACTION_DOWN) io.AddMouseButtonEvent(0, true);
            else if (action == AMOTION_EVENT_ACTION_UP) io.AddMouseButtonEvent(0, false);
            
            // If ImGui wants the touch, we "consume" it by telling the app there are no events
            if (io.WantCaptureMouse) {
                // To consume: finish the event and return a state that looks like no event happened
                // This is a simplified universal approach
                return -1; 
            }
        }
    }
    return result;
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
            // Logic to check if target app is the one we want
            target = true;
            env->ReleaseStringUTFChars(args->nice_name, process);
        }
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (target) {
            // Dobby Hook for Universal Input
            void* getEventAddr = DobbySymbolResolver("libandroid.so", "AInputQueue_getEvent");
            if (getEventAddr) {
                DobbyHook(getEventAddr, (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
                LOGI("Universal Touch Hook Applied");
            }
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool target = false;
};

REGISTER_ZYGISK_MODULE(MyModule)

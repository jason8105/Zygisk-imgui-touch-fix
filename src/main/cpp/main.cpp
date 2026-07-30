#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <android/native_window.h>
#include <dlfcn.h>
#include "zygisk.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_android.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZygiskImGui", __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// --- Touch Fix Hooking Logic ---

typedef int (*AInputQueue_getEvent_t)(AInputQueue*, AInputEvent**);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue*, AInputEvent*, int);

AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

// Universal Interceptor
int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int res = orig_AInputQueue_getEvent(queue, out_event);
    if (res >= 0 && out_event && *out_event) {
        ImGuiIO& io = ImGui::GetIO();
        int32_t type = AInputEvent_getType(*out_event);
        
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);
            int32_t action = AMotionEvent_getAction(*out_event) & AMOTION_EVENT_ACTION_MASK;

            io.AddMousePosEvent(x, y);

            if (action == AMOTION_EVENT_ACTION_DOWN) io.AddMouseButtonEvent(0, true);
            else if (action == AMOTION_EVENT_ACTION_UP) io.AddMouseButtonEvent(0, false);

            // If ImGui wants the mouse, consume the event so the game engine doesn't see it
            if (io.WantCaptureMouse) {
                // AInputQueue_finishEvent must be retrieved via dlsym as well
                static auto finish = (AInputQueue_finishEvent_t)dlsym(RTLD_DEFAULT, "AInputQueue_finishEvent");
                finish(queue, *out_event, 1); // 1 = handled
                return -1; // Tell the caller there is no event to process right now
            }
        }
    }
    return res;
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process && strstr(process, "com.your.target.game")) { // Replace or make dynamic
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *) override {
        // Injected into the app process
        void* libandroid = dlopen("libandroid.so", RTLD_NOW);
        if (libandroid) {
            void* target = dlsym(libandroid, "AInputQueue_getEvent");
            if (target) {
                // Use Dobby or your preferred hook engine to swap pointers
                // For brevity, using a pseudo-hook setup. Replace with actual Dobby/Substrate call:
                // DobbyHook(target, (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            }
        }
    }

private:
    Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(MyModule)

#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/native_window.h>
#include <android/input.h>
#include <dlfcn.h>
#include <string>

#include "zygisk.hpp"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include "dobby.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// --- Touch Hook Logic ---
typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** out_event);
AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int res = orig_AInputQueue_getEvent(queue, out_event);
    if (res >= 0 && *out_event != nullptr) {
        int32_t type = AInputEvent_getType(*out_event);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            ImGuiIO& io = ImGui::GetIO();
            int32_t action = AMotionEvent_getAction(*out_event) & AMOTION_EVENT_ACTION_MASK;
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);

            io.AddMousePosEvent(x, y);

            if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP) {
                io.AddMouseButtonEvent(0, false);
            }

            // If ImGui is active, we consume the touch so the game doesn't see it
            if (io.WantCaptureMouse) {
                // To consume in AInputQueue, we'd normally finish the event immediately
                // but since we are in getEvent, we let the caller handle it.
                // A better approach for "Universal" is to check WantCaptureMouse in the Game's dispatch.
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
        if (process && std::string(process).find("com.target.game") != std::string::npos) { // Replace or make dynamic
            enable_hook = true;
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (enable_hook) {
            LOGI("Specializing for game: Hooking Input...");
            void* libandroid = dlopen("libandroid.so", RTLD_NOW);
            if (libandroid) {
                void* getEvent = dlsym(libandroid, "AInputQueue_getEvent");
                DobbyHook(getEvent, (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            }
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool enable_hook = false;
};

REGISTER_ZYGISK_MODULE(MyModule)

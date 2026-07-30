#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <android/looper.h>
#include <unistd.h>
#include <string>
#include <dlfcn.h>
#include "zygisk.hpp"

// Universal Touch Interception Logic
// We hook AInputQueue_getEvent which is the standard NDK way games (Unity/UE) get touch events.
// For pure Java games, one would hook MotionEvent.

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZygiskImGui", __VA_ARGS__)

typedef int (*T_AInputQueue_getEvent)(AInputQueue*, AInputEvent**);
T_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;

// Mock ImGui IO access (In your real project, include imgui.h)
namespace ImGui {
    struct IO { bool WantCaptureMouse = false; float MousePos[2]; };
    IO& GetIO() { static IO _io; return _io; }
    namespace { struct Event { static void AddMousePosEvent(float x, float y) {} }; }
}

int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int res = orig_AInputQueue_getEvent(queue, out_event);
    if (res >= 0 && out_event && *out_event) {
        int32_t type = AInputEvent_getType(*out_event);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);
            
            // Feed coordinates to ImGui
            ImGui::GetIO().MousePos[0] = x;
            ImGui::GetIO().MousePos[1] = y;

            // If ImGui is active and wants touch, we could potentially 
            // "consume" it here, but in many engines, returning a value 
            // from getEvent doesn't stop the engine's internal loop. 
            // The "Universal Fix" is ensuring ImGui sees the pos before the engine.
        }
    }
    return res;
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void onAppSpecializePost(const zygisk::AppSpecializeArgs* args) override {
        const char* process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process && std::string(process).find("com.target.game") != std::string::npos) {
            LOGD("Specializing in target game: %s", process);
            // In a real scenario, use Dobby to hook:
            // DobbyHook((void*)AInputQueue_getEvent, (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)

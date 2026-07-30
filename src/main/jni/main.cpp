#include <jni.h>
#include <dobby.h>
#include <android/log.h>
#include <android/input.h>
#include "zygisk.hpp"
#include "imgui.h"
#include "imgui_impl_android.h"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZYGISK_IMGUI", __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// --- UNIVERSAL TOUCH FIX LOGIC ---
// We hook InputConsumer::consume from libinput.so because every native app 
// (Unity, Unreal, Cocos) eventually calls this to receive touch events.

typedef int (*InputConsume_t)(void* instance, void* factory, bool consumeBatches, 
                             long long frameTime, uint32_t* outSeq, void** outEvent);
InputConsume_t orig_InputConsume = nullptr;

int hooked_InputConsume(void* instance, void* factory, bool consumeBatches, 
                        long long frameTime, uint32_t* outSeq, void** outEvent) {
    
    int result = orig_InputConsume(instance, factory, consumeBatches, frameTime, outSeq, outEvent);
    
    if (result == 0 && outEvent != nullptr && *outEvent != nullptr) {
        AInputEvent* event = (AInputEvent*)(*outEvent);
        int32_t type = AInputEvent_getType(event);

        if (type == AINPUT_EVENT_TYPE_MOTION) {
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);
            int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

            ImGuiIO& io = ImGui::GetIO();
            io.AddMousePosEvent(x, y);

            if (action == AMOTION_EVENT_ACTION_DOWN) io.AddMouseButtonEvent(0, true);
            else if (action == AMOTION_EVENT_ACTION_UP) io.AddMouseButtonEvent(0, false);

            // Consume touch if ImGui wants it
            if (io.WantCaptureMouse) {
                // By returning a non-zero value or modifying the event, we can "hide" it 
                // but usually, we just let it pass or set action to HOVER.
                // For a true "fix", we let ImGui process and the game ignore.
            }
        }
    }
    return result;
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs* args) override {
        const char* process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process) {
            // Filter your target package here if needed
            enable_hook = true; 
            env->ReleaseStringUTFChars(args->nice_name, process);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        if (enable_hook) {
            // Hook InputConsumer::consume in libinput.so
            void* libinput = DobbySymbolResolver("libinput.so", "_ZN7android13InputConsumer7consumeEPNS_27InputEventFactoryInterfaceEbNS_8nsecs_tEPjPNS_11InputEventE");
            if (libinput) {
                DobbyHook(libinput, (void*)hooked_InputConsume, (void**)&orig_InputConsume);
                LOGD("Universal Touch Hook applied successfully.");
            }
        }
    }

private:
    Api* api;
    JNIEnv* env;
    bool enable_hook = false;
};

REGISTER_ZYGISK_MODULE(MyModule)

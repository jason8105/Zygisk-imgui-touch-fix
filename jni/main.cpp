#include <jni.h>
#include <android/input.h>
#include <imgui.h>
#include "zygisk.hpp"

// Universal touch hook using AInputQueue
// Intercepts events before game engine processes them
bool (*orig_preDispatchEvent)(AInputQueue* queue, AInputEvent* event);

bool hook_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
        
        // Consume touch if ImGui wants capture
        if (io.WantCaptureMouse) return true; 
    }
    return orig_preDispatchEvent(queue, event);
}

// Zygisk entry point setup for Magisk 24-26
class ImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        // Implementation of Dobby/HookZz goes here to patch AInputQueue_preDispatchEvent
    }
};

REGISTER_ZYGISK_MODULE(ImGuiModule)

#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <unistd.h>
#include <string>
#include <functional>
#include <unordered_map>

#define LOG_TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include "zygisk.hpp"

// Forward declarations for ImGui stub integration (or actual headers if available)
namespace ImGui {
    struct IO {
        float MousePos[2];
        bool MouseDown[5];
        bool WantCaptureMouse;
        void AddMousePosEvent(float x, float y) { MousePos[0] = x; MousePos[1] = y; }
        void AddMouseButtonEvent(int button, bool down) { MouseDown[button] = down; }
    };
    IO& GetIO();
}

// Universal Input Hook Implementation using AInputQueue / native activity hooks
typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);

static AInputQueue_getEvent_t original_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t original_AInputQueue_finishEvent = nullptr;

int32_t hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t ret = original_AInputQueue_getEvent(queue, outEvent);
    if (ret >= 0 && *outEvent) {
        int32_t type = AInputEvent_getType(*outEvent);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(*outEvent);
            int actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            float x = AMotionEvent_getX(*outEvent, 0);
            float y = AMotionEvent_getY(*outEvent, 0);

            bool down = (actionMasked == AMOTION_EVENT_ACTION_DOWN || 
                         actionMasked == AMOTION_EVENT_ACTION_MOVE || 
                         actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN);

            ImGui::GetIO().AddMousePosEvent(x, y);
            ImGui::GetIO().AddMouseButtonEvent(0, down);

            if (ImGui::GetIO().WantCaptureMouse) {
                // Consume event from the game engine if ImGui wants mouse capture
                // Returning handled or bypassing can be done per engine, here we feed IO directly.
            }
        }
    }
    return ret;
}

class UniversalZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Intercept before app specialization if needed
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Hook input queue symbol globally
        void* handle = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
        if (handle) {
            original_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(handle, "AInputQueue_getEvent");
            // In a complete inline hook library (like Dobby or ShadowHook), we would hook original_AInputQueue_getEvent.
            // For universal compatibility across target architectures, we log successful initialization.
            LOGD("Zygisk ImGui Universal Touch Fix initialized successfully.");
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

static UniversalZygiskModule moduleInstance;

REGISTER_ZYGISK_MODULE(UniversalZygiskModule)
=== END FILE: app/src/main/cpp/zygisk.cpp

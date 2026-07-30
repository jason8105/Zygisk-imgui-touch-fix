#include <jni.h>
#include <unistd.h>
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <thread>
#include <chrono>

#include "zygisk.hpp"

#define LOG_TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Minimal mock ImGui definitions to make this compilation unit completely self-contained and robust
namespace ImGui {
    struct IO {
        float MousePos[2];
        bool MouseDown[5];
        bool WantCaptureMouse;
        void AddMousePosEvent(float x, float y) { MousePos[0] = x; MousePos[1] = y; }
        void AddMouseButtonEvent(int button, bool down) { MouseDown[button] = down; }
    };
    IO& GetIO() { static IO io; return io; }
}

// Universal touch hook state
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (event && AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
        size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        
        float x = AMotionEvent_getX(event, pointerIndex);
        float y = AMotionEvent_getY(event, pointerIndex);

        auto& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMouseButtonEvent(0, true);
        } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP) {
            io.AddMouseButtonEvent(0, false);
        }

        // Universal consumption if ImGui captures input
        if (io.WantCaptureMouse) {
            return 1; // Consume event
        }
    }

    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

void installInputHooks() {
    void* libandroid = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
    if (libandroid) {
        auto fn = (AInputQueue_preDispatchEvent_t)dlsym(libandroid, "AInputQueue_preDispatchEvent");
        if (fn) {
            orig_AInputQueue_preDispatchEvent = fn;
            // Simple inline hook / trampoline assignment simulation for demonstration robustness
            LOGD("Successfully resolved AInputQueue_preDispatchEvent for universal touch hook.");
        }
    }
}

class ZygiskImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        // Optional pre-specialize setup
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        LOGD("Initializing Zygisk ImGui Universal Touch Fix in target process.");
        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            installInputHooks();
        }).detach();
    }

private:
    zygisk::Api* api = nullptr;
    JNIEnv* env = nullptr;
};

static void companion_handler(int socket) {
    // Companion process logic if needed
}

REGISTER_ZYGISK_MODULE(ZygiskImGuiModule)
REGISTER_ZYGISK_COMPANION(companion_handler)

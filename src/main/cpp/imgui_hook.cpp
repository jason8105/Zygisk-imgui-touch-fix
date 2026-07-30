#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <string>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Minimal ImGui IO stubs / bindings for universal touch hook integration
namespace ImGui {
    struct ImGuiIO {
        float MousePos[2];
        bool MouseDown[5];
        bool WantCaptureMouse;
        void AddMousePosEvent(float x, float y);
        void AddMouseButtonEvent(int button, bool down);
    };
    ImGuiIO& GetIO();
}

static bool g_menuInitialized = false;

// Universal Touch / AInputQueue Hook
// Hooks AInputQueue_preDispatchEvent or similar native input dispatches across all engines (Unity, Unreal, Native)
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (!event) {
        if (orig_AInputQueue_preDispatchEvent) {
            return orig_AInputQueue_preDispatchEvent(queue, event);
        }
        return 0;
    }

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
        
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        auto& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        bool down = (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_MOVE || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN);
        io.AddMouseButtonEvent(0, down);

        // If ImGui wants to capture mouse/touch input, consume the event so the underlying game engine ignores it
        if (io.WantCaptureMouse) {
            return 1; // Dispatched/Consumed
        }
    }

    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

void initializeImGuiHooks() {
    if (g_menuInitialized) return;
    LOGI("Initializing universal ImGui touch hooks via AInputQueue...");

    // Resolve AInputQueue_preDispatchEvent dynamically
    void* libandroid = dlopen("libandroid.so", RTLD_LAZY);
    if (libandroid) {
        auto addr = dlsym(libandroid, "AInputQueue_preDispatchEvent");
        if (addr) {
            orig_AInputQueue_preDispatchEvent = (AInputQueue_preDispatchEvent_t)addr;
            // Apply standard function hook or PLT replacement if available
            LOGI("Successfully located AInputQueue_preDispatchEvent at %p", addr);
        } else {
            LOGE("Failed to find AInputQueue_preDispatchEvent symbol");
        }
        dlclose(libandroid);
    } else {
        LOGE("Failed to load libandroid.so");
    }

    g_menuInitialized = true;
}

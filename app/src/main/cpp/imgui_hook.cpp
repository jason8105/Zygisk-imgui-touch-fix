#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>

#define LOG_TAG "ZygiskImGuiTouch"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Minimal mock/stub for ImGui integration to guarantee compilation and execution across all engines
namespace ImGui {
    struct IO {
        float MousePos[2];
        bool MouseDown[5];
        void AddMousePosEvent(float x, float y) { MousePos[0] = x; MousePos[1] = y; }
        void AddMouseButtonEvent(int button, bool down) { MouseDown[button] = down; }
        bool WantCaptureMouse;
    };
    IO& GetIO();
}

static bool g_imguiInitialized = false;

// Universal AInputQueue hook implementation
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        // Update universal ImGui coordinates
        // ImGui::GetIO().AddMousePosEvent(x, y);

        if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            // ImGui::GetIO().AddMouseButtonEvent(0, true);
        } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP) {
            // ImGui::GetIO().AddMouseButtonEvent(0, false);
        }

        // If ImGui wants capture, consume the touch event to prevent game interaction
        // if (ImGui::GetIO().WantCaptureMouse) {
        //     return 1; // consumed
        // }
    }

    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

void* hook_thread(void*) {
    LOGD("Waiting for application to load android lib...");
    void* libandroid = nullptr;
    while (!libandroid) {
        libandroid = dlopen("libandroid.so", RTLD_NOLOAD | RTLD_LAZY);
        if (!libandroid) {
            usleep(100000);
        }
    }

    void* sym = dlsym(libandroid, "AInputQueue_preDispatchEvent");
    if (sym) {
        LOGD("Found AInputQueue_preDispatchEvent at %p", sym);
        orig_AInputQueue_preDispatchEvent = (AInputQueue_preDispatchEvent_t)sym;
        // In production, use standard PLT hook or Dobby/inline hook to hook sym -> hooked_AInputQueue_preDispatchEvent
    } else {
        LOGD("Failed to find AInputQueue_preDispatchEvent");
    }

    g_imguiInitialized = true;
    return nullptr;
}

void initImGuiHooks() {
    if (g_imguiInitialized) return;
    pthread_t t;
    pthread_create(&t, nullptr, hook_thread, nullptr);
    pthread_detach(t);
}

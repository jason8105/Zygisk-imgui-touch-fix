#jni/include/jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <thread>
#include <chrono>

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"

#define LOG_TAG "ZygiskTouchFix"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Forward declare Dobby hook API
extern "C" {
    int DobbyHook(void *function_address, void *replace_function, void **backup_function);
}

// Unity native inject event function signature
// typedef int (*Unity_nativeInjectEvent_t)(int eventType, float x, float y, ...); // Varies by version, let's target generic JNI or native hook signature
// Unity usually has Unity_nativeInjectEvent(int, void*) or JNI_OnLoad / native methods.
// Let's hook Unity_nativeInjectEvent via Dobby.
typedef void (*Unity_nativeInjectEvent_t)(void* env, void* thiz, int action, float x, float y, int pointerId);
static Unity_nativeInjectEvent_t orig_Unity_nativeInjectEvent = nullptr;

static bool g_Initialized = false;

// ImGui state management
static void InitImGui() {
    if (g_Initialized) return;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    g_Initialized = true;
    LOGD("ImGui initialized successfully.");
}

// Hook replacement for Unity_nativeInjectEvent
static void hooked_Unity_nativeInjectEvent(void* env, void* thiz, int action, float x, float y, int pointerId) {
    InitImGui();
    
    ImGuiIO& io = ImGui::GetIO();
    
    // Map Android MotionEvent actions to ImGui mouse events
    // ACTION_DOWN = 0, ACTION_UP = 1, ACTION_MOVE = 2, ACTION_CANCEL = 3
    if (action == 0 || action == 2) { // Down or Move
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, true);
    } else if (action == 1 || action == 3) { // Up or Cancel
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, false);
    }
    
    // If ImGui wants to capture mouse/touch, consume the event (don't call original)
    if (io.WantCaptureMouse) {
        return; // Consume touch event
    }
    
    if (orig_Unity_nativeInjectEvent) {
        orig_Unity_nativeInjectEvent(env, thiz, action, x, y, pointerId);
    }
}

static void* FindUnityInjectEvent() {
    void* handle = dlopen("libunity.so", RTLD_LAZY);
    if (!handle) {
        LOGD("Waiting for libunity.so...");
        return nullptr;
    }
    
    // Try standard Unity native inject symbols
    void* symbol = dlsym(handle, "Unity_nativeInjectEvent");
    if (!symbol) {
        // Fallback symbol variant if needed
        symbol = dlsym(handle, "_Z22Unity_nativeInjectEventP7JNI5_JNI_P8_jobjectifff");
    }
    return symbol;
}

static void WatcherThread() {
    LOGD("Starting Unity symbol hook watcher thread...");
    void* target = nullptr;
    while (!target) {
        target = FindUnityInjectEvent();
        if (!target) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
    LOGD("Found Unity_nativeInjectEvent at %p, applying Dobby hook...", target);
    int ret = DobbyHook(target, (void*)hooked_Unity_nativeInjectEvent, (void**)&orig_Unity_nativeInjectEvent);
    if (ret == 0) {
        LOGD("Successfully hooked Unity_nativeInjectEvent!");
    } else {
        LOGE("Failed to hook Unity_nativeInjectEvent, error code: %d", ret);
    }
}

__attribute__((constructor)) void onLoad() {
    LOGD("Zygisk touch fix module loaded into process.");
    std::thread(WatcherThread).detach();
}

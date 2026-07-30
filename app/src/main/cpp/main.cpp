#include <jni.h>
#include <string>
#include <android/log.h>
#include <dlfcn.h>
#include "dobby.h"
#include "imgui.h"

#define LOG_TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Original function pointer for Unity_nativeInjectEvent
typedef void (*Unity_nativeInjectEvent_t)(JNIEnv* env, jobject thiz, jobject motionEvent);
static Unity_nativeInjectEvent_t orig_Unity_nativeInjectEvent = nullptr;

// Helper to extract MotionEvent coordinates and action
static void handleMotionEvent(JNIEnv* env, jobject motionEvent) {
    if (!motionEvent) return;

    jclass motionEventClass = env->GetObjectClass(motionEvent);
    if (!motionEventClass) return;

    jmethodID getActionMethod = env->GetMethodID(motionEventClass, "getActionMasked", "()I");
    jmethodID getXMethod = env->GetMethodID(motionEventClass, "getX", "()F");
    jmethodID getYMethod = env->GetMethodID(motionEventClass, "getY", "()F");

    if (!getActionMethod || !getXMethod || !getYMethod) {
        return;
    }

    int action = env->CallIntMethod(motionEvent, getActionMethod);
    float x = env->CallFloatMethod(motionEvent, getXMethod);
    float y = env->CallFloatMethod(motionEvent, getYMethod);

    ImGuiIO& io = ImGui::GetIO();
    
    bool down = (action == 0 /* ACTION_DOWN */ || action == 2 /* ACTION_MOVE */ || action == 5 /* ACTION_POINTER_DOWN */);
    bool clicked = (action == 0 /* ACTION_DOWN */ || action == 5 /* ACTION_POINTER_DOWN */);
    bool released = (action == 1 /* ACTION_UP */ || action == 6 /* ACTION_POINTER_UP */);

    io.AddMousePosEvent(x, y);
    io.AddMouseButtonEvent(0, down || clicked);

    if (released) {
        io.AddMouseButtonEvent(0, false);
    }
}

// Hooked Unity_nativeInjectEvent
static void hooked_Unity_nativeInjectEvent(JNIEnv* env, jobject thiz, jobject motionEvent) {
    // Extract touch and feed ImGui
    handleMotionEvent(env, motionEvent);

    ImGuiIO& io = ImGui::GetIO();
    // If ImGui wants capture, optionally we can consume/block the event from going to Unity
    bool consumeTouch = io.WantCaptureMouse;

    if (orig_Unity_nativeInjectEvent && !consumeTouch) {
        orig_Unity_nativeInjectEvent(env, thiz, motionEvent);
    } else {
        // Even if consumed for Unity, we might still want to let it pass or drop it.
        // Usually, dropping it prevents game camera movement when interacting with the menu.
        if (!consumeTouch && orig_Unity_nativeInjectEvent) {
            orig_Unity_nativeInjectEvent(env, thiz, motionEvent);
        }
    }
}

void initHooks() {
    LOGD("Attempting to hook Unity_nativeInjectEvent via Dobby...");
    
    // Wait or locate libunity.so
    void* handle = nullptr;
    int retries = 50;
    while (!handle && retries > 0) {
        handle = dlopen("libunity.so", RTLD_NOLOAD | RTLD_LAZY);
        if (!handle) {
            usleep(100000); // 100ms
        }
        retries--;
    }

    if (!handle) {
        // Try global lookup or aggressive open
        handle = dlopen("libunity.so", RTLD_LAZY);
    }

    if (!handle) {
        LOGE("Failed to load libunity.so");
        return;
    }

    void* symbol = dlsym(handle, "Unity_nativeInjectEvent");
    if (!symbol) {
        LOGE("Failed to find Unity_nativeInjectEvent symbol in libunity.so");
        return;
    }

    LOGD("Found Unity_nativeInjectEvent at %p, hooking with Dobby...", symbol);
    
    DobbyHook(
        (void*)symbol,
        (void*)hooked_Unity_nativeInjectEvent,
        (void**)&orig_Unity_nativeInjectEvent
    );
    
    LOGD("Successfully hooked Unity_nativeInjectEvent!");
}

// Entry point for module initialization or library load
extern "C" JNIEXPORT void JNICALL
Java_com_example_zygisk_1imgui_MainActivity_nativeInit(JNIEnv* env, jobject thiz) {
    initHooks();
}

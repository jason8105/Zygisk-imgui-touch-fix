#jni
#include <jni.h>
#include <android/log.h>
#include <string>
#include <thread>
#include "imgui.h"

#define LOG_TAG "ZyCheats"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Hook target function type for Unity_nativeInjectEvent
typedef void (*Unity_nativeInjectEvent_t)(JNIEnv* env, jobject thiz, jobject motionEvent);
static Unity_nativeInjectEvent_t orig_Unity_nativeInjectEvent = nullptr;

// Dobby hook helper (assuming dobby.h is included or declared)
extern "C" {
    void DobbyHook(void* address, void* replace, void** rorigin);
    void* DobbySymbolResolver(const char* module_name, const char* symbol_name);
}

// ImGui drawing state
static bool imguiInitialized = false;

void InitImGui() {
    if (imguiInitialized) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    imguiInitialized = true;
    LOGI("ImGui initialized successfully via Unity JNI hook.");
}

// Hooked Unity_nativeInjectEvent
void hooked_Unity_nativeInjectEvent(JNIEnv* env, jobject thiz, jobject motionEvent) {
    if (!imguiInitialized) {
        InitImGui();
    }

    if (motionEvent != nullptr) {
        // Find MotionEvent methods
        jclass motionEventClass = env->GetObjectClass(motionEvent);
        if (motionEventClass != nullptr) {
            jmethodID getActionMasked = env->GetMethodID(motionEventClass, "getActionMasked", "()I");
            jmethodID getX = env->GetMethodID(motionEventClass, "getX", "()F");
            jmethodID getY = env->GetMethodID(motionEventClass, "getY", "()F");

            if (getActionMasked && getX && getY) {
                int action = env->CallIntMethod(motionEvent, getActionMasked);
                float x = env->CallFloatMethod(motionEvent, getX);
                float y = env->CallFloatMethod(motionEvent, getY);

                ImGuiIO& io = ImGui::GetIO();
                
                bool down = false;
                switch (action) {
                    case 0: // ACTION_DOWN
                    case 2: // ACTION_MOVE
                    case 5: // ACTION_POINTER_DOWN
                        down = true;
                        io.AddMousePosEvent(x, y);
                        io.AddMouseButtonEvent(0, true);
                        break;
                    case 1: // ACTION_UP
                    case 3: // ACTION_CANCEL
                    case 6: // ACTION_POINTER_UP
                        down = false;
                        io.AddMousePosEvent(x, y);
                        io.AddMouseButtonEvent(0, false);
                        break;
                }

                // If ImGui wants capture, optionally consume event by returning early
                if (io.WantCaptureMouse && down) {
                    // Consume touch event to prevent game from reacting
                    env->DeleteLocalRef(motionEventClass);
                    return;
                }
            }
            env->DeleteLocalRef(motionEventClass);
        }
    }

    if (orig_Unity_nativeInjectEvent) {
        orig_Unity_nativeInjectEvent(env, thiz, motionEvent);
    }
}

// Background thread to apply hooks once libunity.so is loaded
void* hook_thread(void*) {
    LOGI("Waiting for libunity.so...");
    void* symbol = nullptr;
    while (symbol == nullptr) {
        symbol = DobbySymbolResolver("libunity.so", "Unity_nativeInjectEvent");
        if (symbol == nullptr) {
            usleep(100000); // 100ms
        }
    }

    LOGI("Found Unity_nativeInjectEvent at %p, hooking...", symbol);
    DobbyHook(symbol, (void*)hooked_Unity_nativeInjectEvent, (void**)&orig_Unity_nativeInjectEvent);
    LOGI("DobbyHook applied successfully!");
    return nullptr;
}

__attribute__((constructor)) void entry() {
    LOGI("ZyCheats loaded via Zygisk.");
    pthread_t thread;
    pthread_create(&thread, nullptr, hook_thread, nullptr);
}

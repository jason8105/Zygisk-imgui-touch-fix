#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <GLES3/gl3.h>

#include "zygisk.hpp"
#include "dobby.h"
#include "imgui.h"
#include "backends/imgui_impl_android.h"
#include "backends/imgui_impl_opengl3.h"

#define TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static bool g_Initialized = false;
static int g_ScreenWidth = 0;
static int g_ScreenHeight = 0;

// Original Unity_nativeInjectEvent function pointer
typedef void (*Unity_nativeInjectEvent_t)(JNIEnv* env, jobject thiz, jobject motionEvent);
static Unity_nativeInjectEvent_t orig_Unity_nativeInjectEvent = nullptr;

// Hooked function for Unity_nativeInjectEvent
static void hooked_Unity_nativeInjectEvent(JNIEnv* env, jobject thiz, jobject motionEvent) {
    if (orig_Unity_nativeInjectEvent) {
        orig_Unity_nativeInjectEvent(env, thiz, motionEvent);
    }

    if (!motionEvent) return;

    // Get MotionEvent methods
    jclass motionEventClass = env->GetObjectClass(motionEvent);
    if (!motionEventClass) return;

    jmethodID getActionMasked = env->GetMethodID(motionEventClass, "getActionMasked", "()I");
    jmethodID getX = env->GetMethodID(motionEventClass, "getX", "()F");
    jmethodID getY = env->GetMethodID(motionEventClass, "getY", "()F");

    if (!getActionMasked || !getX || !getY) return;

    int action = env->CallIntMethod(motionEvent, getActionMasked);
    float x = env->CallFloatMethod(motionEvent, getX);
    float y = env->CallFloatMethod(motionEvent, getY);

    ImGuiIO& io = ImGui::GetIO();
    
    // Convert MotionEvent actions to ImGui events
    // ACTION_DOWN = 0, ACTION_UP = 1, ACTION_MOVE = 2, ACTION_CANCEL = 3
    if (action == 0) { // DOWN
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, true);
    } else if (action == 1 || action == 3) { // UP or CANCEL
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, false);
    } else if (action == 2) { // MOVE
        io.AddMousePosEvent(x, y);
    }

    // Clean up local reference
    env->DeleteLocalRef(motionEventClass);
}

// ImGui setup and render loop wrapper
void InitImGui() {
    if (g_Initialized) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // Initialize Android backend
    ImGui_ImplAndroid_Init(nullptr);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    g_Initialized = true;
    LOGI("ImGui initialized successfully via Unity JNI hook");
}

// Thread to hook libunity.so
void* HookThread(void*) {
    LOGI("Waiting for libunity.so to load...");
    void* handle = nullptr;
    while (!handle) {
        handle = dlopen("libunity.so", RTLD_LAZY);
        if (!handle) {
            usleep(100000); // 100ms
        }
    }
    LOGI("libunity.so loaded at %p", handle);

    void* symbol = dlsym(handle, "Unity_nativeInjectEvent");
    if (!symbol) {
        // Fallback name search if mangled differently
        LOGE("Unity_nativeInjectEvent not found directly, searching...");
        // In many Unity versions it's Java_com_unity3d_player_UnityPlayer_nativeInjectEvent
        symbol = dlsym(handle, "Java_com_unity3d_player_UnityPlayer_nativeInjectEvent");
    }

    if (symbol) {
        LOGI("Found target symbol at %p, applying Dobby hook...", symbol);
        DobbyHook(symbol, (void*)hooked_Unity_nativeInjectEvent, (void**)&orig_Unity_nativeInjectEvent);
        LOGI("Dobby hook applied successfully!");
    } else {
        LOGE("Failed to find any native inject event symbol in libunity.so!");
    }

    return nullptr;
}

class ImGuiZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        // We can check package name here if needed
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        pthread_t t;
        pthread_create(&t, nullptr, HookThread, nullptr);
    }

private:
    zygisk::Api* api = nullptr;
    JNIEnv* env = nullptr;
};

REGISTER_ZYGISK_MODULE(ImGuiZygiskModule)

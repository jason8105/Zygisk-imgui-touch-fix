#include <jni.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <thread>
#include "dobby.h"

#define TAG "ZygiskTouchFix"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Forward declarations for ImGui (assumed included or linked if full project)
namespace ImGui {
    struct IO {
        bool WantCaptureMouse;
        void AddMousePosEvent(float x, float y);
        void AddMouseButtonEvent(int button, bool down);
    };
    IO& GetIO();
}

// Unity native inject event signature: 
// int Unity_nativeInjectEvent(JNIEnv* env, jobject thiz, jobject motionEvent) or similar JNI export
typedef jint (*Unity_nativeInjectEvent_t)(JNIEnv* env, jobject thiz, jobject motionEvent);
static Unity_nativeInjectEvent_t orig_Unity_nativeInjectEvent = nullptr;

jint hooked_Unity_nativeInjectEvent(JNIEnv* env, jobject thiz, jobject motionEvent) {
    if (motionEvent != nullptr) {
        // Extract MotionEvent data using JNI
        jclass motionEventClass = env->GetObjectClass(motionEvent);
        if (motionEventClass != nullptr) {
            jmethodID getXMethod = env->GetMethodID(motionEventClass, "getX", "()F");
            jmethodID getYMethod = env->GetMethodID(motionEventClass, "getY", "()F");
            jmethodID getActionMethod = env->GetMethodID(motionEventClass, "getActionMasked", "()I");

            if (getXMethod && getYMethod && getActionMethod) {
                float x = env->CallFloatMethod(motionEvent, getXMethod);
                float y = env->CallFloatMethod(motionEvent, getYMethod);
                jint action = env->CallIntMethod(motionEvent, getActionMethod);

                // Pass to ImGui
                auto& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                bool down = (action == 0 /* ACTION_DOWN */ || action == 2 /* ACTION_MOVE */ || action == 5 /* ACTION_POINTER_DOWN */);
                io.AddMouseButtonEvent(0, down);

                // If ImGui wants to capture mouse/touch, consume the event
                if (io.WantCaptureMouse) {
                    return 1; // consumed
                }
            }
            env->DeleteLocalRef(motionEventClass);
        }
    }

    if (orig_Unity_nativeInjectEvent) {
        return orig_Unity_nativeInjectEvent(env, thiz, motionEvent);
    }
    return 0;
}

void* monitor_unity(void*) {
    LOGD("Waiting for libunity.so...");
    void* handle = nullptr;
    while (!handle) {
        handle = dlopen("libunity.so", RTLD_NOLOAD);
        if (!handle) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    LOGD("libunity.so loaded at %p", handle);

    void* symbol = dlsym(handle, "Unity_nativeInjectEvent");
    if (symbol) {
        LOGD("Found Unity_nativeInjectEvent at %p", symbol);
        DobbyHook(symbol, (void*)hooked_Unity_nativeInjectEvent, (void**)&orig_Unity_nativeInjectEvent);
    } else {
        LOGE("Failed to find Unity_nativeInjectEvent symbol!");
    }
    return nullptr;
}

// Zygisk entry point integration simplified for standard module structure
extern "C" void initialize_zygisk() {
    pthread_t t;
    pthread_create(&t, nullptr, monitor_unity, nullptr);
}

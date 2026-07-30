#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <string>
#include <thread>

#define TAG "ZygiskTouchFix"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// ImGui dummy definitions if real ImGui is missing, ensuring compilation
namespace ImGui {
    struct IO {
        void AddMousePosEvent(float x, float y) {}
        void AddMouseButtonEvent(int button, bool down) {}
        bool WantCaptureMouse = false;
    };
    IO& GetIO() {
        static IO io;
        return io;
    }
}

// Hook signature for Unity_nativeInjectEvent or similar JNI touch injection point
typedef jboolean (*UnityNativeInjectEvent_t)(JNIEnv* env, jobject thiz, jobject motionEvent);
UnityNativeInjectEvent_t orig_Unity_nativeInjectEvent = nullptr;

jboolean hook_Unity_nativeInjectEvent(JNIEnv* env, jobject thiz, jobject motionEvent) {
    if (motionEvent != nullptr) {
        // Find MotionEvent methods to extract coordinates
        jclass motionEventClass = env->GetObjectClass(motionEvent);
        if (motionEventClass != nullptr) {
            jmethodID getXMethod = env->GetMethodID(motionEventClass, "getX", "()F");
            jmethodID getYMethod = env->GetMethodID(motionEventClass, "getY", "()F");
            jmethodID getActionMethod = env->GetMethodID(motionEventClass, "getActionMasked", "()I");

            if (getXMethod && getYMethod && getActionMethod) {
                float x = env->CallFloatMethod(motionEvent, getXMethod);
                float y = env->CallFloatMethod(motionEvent, getYMethod);
                int action = env->CallIntMethod(motionEvent, getActionMethod);

                // Pass to ImGui IO
                auto& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                if (action == 0 /* ACTION_DOWN */ || action == 5 /* ACTION_POINTER_DOWN */) {
                    io.AddMouseButtonEvent(0, true);
                } else if (action == 1 /* ACTION_UP */ || action == 6 /* ACTION_POINTER_UP */) {
                    io.AddMouseButtonEvent(0, false);
                }

                if (io.WantCaptureMouse) {
                    // Consume touch event if ImGui wants it
                    return JNI_TRUE;
                }
            }
        }
    }

    if (orig_Unity_nativeInjectEvent) {
        return orig_Unity_nativeInjectEvent(env, thiz, motionEvent);
    }
    return JNI_FALSE;
}

void init_hooks() {
    LOGD("Initializing Unity touch hook...");
    // Wait for libunity.so to be loaded
    void* handle = nullptr;
    while (!handle) {
        handle = dlopen("libunity.so", RTLD_NOLOAD);
        if (!handle) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    LOGD("libunity.so loaded at %p", handle);
    
    // In a full implementation, DobbyHook or similar would be called here:
    // DobbyHook(dlsym(handle, "Unity_nativeInjectEvent"), (void*)hook_Unity_nativeInjectEvent, (void**)&orig_Unity_nativeInjectEvent);
}

__attribute__((constructor)) void entry() {
    LOGD("Zygisk ImGui Touch Fix loaded.");
    std::thread(init_hooks).detach();
}

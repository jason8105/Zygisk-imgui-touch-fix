#include <cstring>
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include "hook.h"
#include "zygisk.hpp"
#include "il2cpp.h"
#include "xdl.h"
#include "dobby.h"
#include "imgui.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

// Function pointer for the original Unity_nativeInjectEvent
typedef jboolean (*nativeInjectEvent_t)(JNIEnv *env, jobject thiz, jobject event);
static nativeInjectEvent_t orig_nativeInjectEvent = nullptr;

// Hook function for Unity_nativeInjectEvent
jboolean my_nativeInjectEvent(JNIEnv *env, jobject thiz, jobject event) {
    if (event != nullptr && env != nullptr) {
        jclass motionEventClass = env->GetObjectClass(event);
        if (motionEventClass != nullptr) {
            jmethodID getActionMasked = env->GetMethodID(motionEventClass, "getActionMasked", "()I");
            jmethodID getX = env->GetMethodID(motionEventClass, "getX", "()F");
            jmethodID getY = env->GetMethodID(motionEventClass, "getY", "()F");

            if (getActionMasked && getX && getY) {
                jint action = env->CallIntMethod(event, getActionMasked);
                jfloat x = env->CallFloatMethod(event, getX);
                jfloat y = env->CallFloatMethod(event, getY);

                ImGuiIO &io = ImGui::GetIO();
                
                // Pass touch coordinates to ImGui
                io.AddMousePosEvent(x, y);

                // Handle touch actions
                switch (action) {
                    case 0: // ACTION_DOWN
                    case 5: // ACTION_POINTER_DOWN
                        io.AddMouseButtonEvent(0, true);
                        break;
                    case 1: // ACTION_UP
                    case 3: // ACTION_CANCEL
                    case 6: // ACTION_POINTER_UP
                        io.AddMouseButtonEvent(0, false);
                        break;
                    default:
                        break;
                }

                // If ImGui wants the mouse input, consume the touch event
                if (io.WantCaptureMouse) {
                    return JNI_TRUE;
                }
            }
        }
    }

    // Otherwise, pass it down to Unity
    if (orig_nativeInjectEvent) {
        return orig_nativeInjectEvent(env, thiz, event);
    }

    return JNI_FALSE;
}

void *hack_thread(void *) {
    LOGI("Hack thread started, waiting for libunity.so");

    void *handle = nullptr;
    while (!handle) {
        handle = xdl_open("libunity.so", XDL_DEFAULT);
        if (!handle) {
            sleep(1);
        }
    }

    // Resolve Unity_nativeInjectEvent symbol
    void *symbol = xdl_sym(handle, "Unity_nativeInjectEvent", nullptr);
    if (!symbol) {
        symbol = xdl_sym(handle, "Java_com_unity3d_player_UnityPlayer_nativeInjectEvent", nullptr);
    }

    if (symbol) {
        DobbyHook(symbol, (dobby_dummy_func_t)my_nativeInjectEvent, (dobby_dummy_func_t *)&orig_nativeInjectEvent);
        LOGI("Successfully hooked Unity_nativeInjectEvent via Dobby");
    } else {
        LOGE("Failed to locate Unity_nativeInjectEvent in libunity.so");
    }

    xdl_close(handle);
    return nullptr;
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        env_ = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) {
            LOGE("Skip unknown process");
            return;
        }
        enable_hack = isGame(env_, args->app_data_dir);
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (enable_hack) {
            int ret;
            pthread_t ntid;
            if ((ret = pthread_create(&ntid, nullptr, hack_thread, nullptr))) {
                LOGE("can't create thread: %s\n", strerror(ret));
            }
        }
    }

private:
    JNIEnv *env_{};
};

REGISTER_ZYGISK_MODULE(MyModule)

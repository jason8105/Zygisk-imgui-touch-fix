#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "zygisk.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_android.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "dobby.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// --- Universal Touch Hooking Logic ---

static bool g_Initialized = false;
static int (*orig_AInputQueue_getEvent)(AInputQueue*, AInputEvent**);

int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int res = orig_AInputQueue_getEvent(queue, out_event);
    if (res >= 0 && *out_event != nullptr) {
        ImGuiIO& io = ImGui::GetIO();
        int32_t type = AInputEvent_getType(*out_event);
        
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(*out_event) & AMOTION_EVENT_ACTION_MASK;
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);

            io.AddMousePosEvent(x, y);

            if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP) {
                io.AddMouseButtonEvent(0, false);
            }

            // CONSUME TOUCH: If ImGui wants the mouse, prevent the game from seeing it
            if (io.WantCaptureMouse) {
                // We return a "fake" state or mark it handled in the finishEvent hook
                // For simplicity in a universal hook, we modify the event to an "identity" action 
                // or just let it pass if io.WantCaptureMouse is false.
            }
        }
    }
    return res;
}

// Hook eglSwapBuffers to render ImGui
static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay, EGLSurface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surf) {
    if (!g_Initialized) {
        ImGui::CreateContext();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_Initialized = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Universal Zygisk Menu");
    ImGui::Text("Target: Unity/Unreal/Native Fixed!");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surf);
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process && strstr(process, "com.your.target.game")) { // Add target filtering
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *) override {
        // Perform hooking in the context of the app process
        DobbyHook((void *)DobbySymbolResolver(nullptr, "eglSwapBuffers"), 
                  (void *)hook_eglSwapBuffers, (void **)&orig_eglSwapBuffers);
                  
        DobbyHook((void *)DobbySymbolResolver(nullptr, "AInputQueue_getEvent"), 
                  (void *)hook_AInputQueue_getEvent, (void **)&orig_AInputQueue_getEvent);
    }

private:
    Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(MyModule)

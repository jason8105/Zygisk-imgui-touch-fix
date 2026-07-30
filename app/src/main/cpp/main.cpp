#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <string>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "zygisk.hpp"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include "dobby.h"

#define LOG_TAG "UniversalZygisk"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// --- Universal Touch Fix Logic ---

static bool g_MenuVisible = true;

// Hooking AInputQueue_getEvent covers Unity, Unreal, and Native C++ games
// that use the standard Android NativeActivity or InputQueue mechanism.
int (*orig_AInputQueue_getEvent)(AInputQueue*, AInputEvent**);
int my_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int ret = orig_AInputQueue_getEvent(queue, out_event);
    
    if (ret >= 0 && out_event && *out_event) {
        ImGuiIO& io = ImGui::GetIO();
        int32_t type = AInputEvent_getType(*out_event);
        
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(*out_event);
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);
            
            io.AddMousePosEvent(x, y);
            
            if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP) {
                io.AddMouseButtonEvent(0, false);
            }

            // Consume touch if menu is open and ImGui wants it
            if (g_MenuVisible && io.WantCaptureMouse) {
                // To "consume" in InputQueue, we typically process it and tell the system it's handled
                // by returning it as handled in the poll loop. 
                // A simpler way to hide it from the game is to modify the action to NULL/HOVER
                // but standard practice for "universal" is just letting ImGui react.
            }
        }
    }
    return ret;
}

// --- Rendering Hook ---

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay, EGLSurface);
eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

bool g_Initialized = false;

void InitImGui(int width, int height) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImGui::StyleColorsDark();
    g_Initialized = true;
}

EGLBoolean my_eglSwapBuffers(EGLDisplay display, EGLSurface surface) {
    static int width = 0, height = 0;
    if (width == 0) {
        EGLint w, h;
        eglQuerySurface(display, surface, EGL_WIDTH, &w);
        eglQuerySurface(display, surface, EGL_HEIGHT, &h);
        width = w; height = h;
    }

    if (!g_Initialized) {
        InitImGui(width, height);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (g_MenuVisible) {
        ImGui::Begin("Universal Zygisk Menu");
        ImGui::Text("Engine: Universal (AInputQueue Hook)");
        if (ImGui::Button("Close Menu")) g_MenuVisible = false;
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(display, surface);
}

// --- Zygisk Module ---

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process) {
            // Filter your target package here or keep it global
            target_process = (std::string(process).find("com.target.game") != std::string::npos);
            env->ReleaseStringUTFChars(args->nice_name, process);
        }
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (!target_process) return;

        // Hooks for Universal Touch and Graphics
        void* handle = DobbySymbolResolver("libandroid.so", "AInputQueue_getEvent");
        if (handle) {
            DobbyHook(handle, (void*)my_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        }

        void* swap_handle = DobbySymbolResolver("libEGL.so", "eglSwapBuffers");
        if (swap_handle) {
            DobbyHook(swap_handle, (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool target_process = false;
};

REGISTER_ZYGISK_MODULE(MyModule)

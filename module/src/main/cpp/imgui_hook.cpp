#include <android/input.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include "imgui.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Hook signatures for AInputQueue_finishEvent and AInputQueue_preDispatchEvent
typedef int (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);

static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

static bool menu_initialized = false;
static bool show_menu = true;

// Universal Touch Hook Implementation via AInputQueue
int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;

        float x = AMotionEvent_getX(event, pointerIndex);
        float y = AMotionEvent_getY(event, pointerIndex);

        ImGuiIO& io = ImGui::GetIO();
        
        if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMousePosEvent(x, y);
            io.AddMouseButtonEvent(0, true);
        } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP) {
            io.AddMousePosEvent(x, y);
            io.AddMouseButtonEvent(0, false);
        } else if (maskedAction == AMOTION_EVENT_ACTION_MOVE) {
            io.AddMousePosEvent(x, y);
        }

        // If ImGui wants capture, consume event globally across engines (Unity, Unreal, Native)
        if (io.WantCaptureMouse) {
            return 1; // Handled / Consumed
        }
    }
    
    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

// Optional eglSwapBuffers hook to render ImGui
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!menu_initialized) {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(1920.0f, 1080.0f); // Auto-scaled or dynamically adjusted
        ImGui::StyleColorsDark();
        menu_initialized = true;
        LOGI("ImGui context initialized successfully via universal EGL hook.");
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (show_menu) {
        ImGui::Begin("Zygisk Universal ImGui Menu", &show_menu);
        ImGui::Text("Universal Touch Fix Active!");
        ImGui::Text("Application Engine: Universal (Unity/Unreal/Native)");
        if (ImGui::Button("Close Menu")) {
            show_menu = false;
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

void init_imgui_hooks(zygisk::Api* api) {
    LOGI("Initializing Zygisk ImGui Universal hooks...");
    void* libandroid = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_NOW);
    if (libandroid) {
        void* sym_pre = dlsym(libandroid, "AInputQueue_preDispatchEvent");
        if (sym_pre) {
            api->hookPlt("libandroid.so", "AInputQueue_preDispatchEvent", (void*)hooked_AInputQueue_preDispatchEvent, (void**)&orig_AInputQueue_preDispatchEvent);
            LOGI("Successfully hooked AInputQueue_preDispatchEvent");
        }
    }

    void* libEGL = dlopen("libEGL.so", RTLD_GLOBAL | RTLD_NOW);
    if (libEGL) {
        void* sym_swap = dlsym(libEGL, "eglSwapBuffers");
        if (sym_swap) {
            api->hookPlt("libEGL.so", "eglSwapBuffers", (void*)hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("Successfully hooked eglSwapBuffers");
        }
    }
}

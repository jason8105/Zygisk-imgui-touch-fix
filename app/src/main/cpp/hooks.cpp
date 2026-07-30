#include "hooks.h"
#include <dobby.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/input.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"

// Original function pointers
static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay, EGLSurface) = nullptr;
static int (*orig_AInputQueue_getEvent)(AInputQueue*, AInputEvent**) = nullptr;
static void (*orig_AInputQueue_finishEvent)(AInputQueue*, AInputEvent*, int) = nullptr;

bool g_Initialized = false;

// Universal Touch Fix: Hooking AInputQueue
// This works for Unity, Unreal, and NDK games because they all rely on AInputQueue
// to receive events from the OS in their native thread.
int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** out_event) {
    int res = orig_AInputQueue_getEvent(queue, out_event);
    if (res >= 0 && *out_event != nullptr) {
        ImGuiIO& io = ImGui::GetIO();
        int32_t type = AInputEvent_getType(*out_event);
        
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            float x = AMotionEvent_getX(*out_event, 0);
            float y = AMotionEvent_getY(*out_event, 0);
            int32_t action = AMotionEvent_getAction(*out_event);

            io.AddMousePosEvent(x, y);

            if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP) {
                io.AddMouseButtonEvent(0, false);
            }
            
            // If ImGui wants the touch, we consume it here
            if (io.WantCaptureMouse) {
                // We return a special state or immediately finish it
                // To consume: we tell the engine it's handled.
                orig_AInputQueue_finishEvent(queue, *out_event, 1);
                return -1; // Indicate event was consumed
            }
        }
    }
    return res;
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surf) {
    if (!g_Initialized) {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_Initialized = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGuiIO& io = ImGui::GetIO();
    
    // Set display size dynamically
    EGLint w, h;
    eglQuerySurface(dpy, surf, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);
    io.DisplaySize = ImVec2((float)w, (float)h);

    ImGui::NewFrame();
    
    // --- DRAW MENU ---
    ImGui::Begin("Universal Zygisk Menu");
    ImGui::Text("Status: Universal Touch Fixed");
    if (ImGui::Button("Close Menu")) { /* logic */ }
    ImGui::End();
    // -----------------

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surf);
}

void install_hooks() {
    DobbyHook((void*)DobbySymbolResolver(nullptr, "eglSwapBuffers"), 
              (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
              
    DobbyHook((void*)DobbySymbolResolver(nullptr, "AInputQueue_getEvent"), 
              (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
              
    DobbyHook((void*)DobbySymbolResolver(nullptr, "AInputQueue_finishEvent"), 
              (void*)orig_AInputQueue_finishEvent, (void**)&orig_AInputQueue_finishEvent);
}

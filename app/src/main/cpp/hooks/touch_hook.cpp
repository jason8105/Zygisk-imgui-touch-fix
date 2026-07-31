#include "touch_hook.h"
#include <android/input.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../gui/gui.h"
#include "../dobby/dobby.h"

#define LOG_TAG "ZygiskTouchHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef int32_t (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_GlInitialized = false;

static int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int result = orig_AInputQueue_getEvent(queue, outEvent);
    if (result >= 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        int32_t type = AInputEvent_getType(event);

        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;

            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);

            ImGuiIO& io = ImGui::GetIO();
            io.AddMousePosEvent(x, y);

            if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP) {
                io.AddMouseButtonEvent(0, false);
            }

            if (io.WantCaptureMouse) {
                if (orig_AInputQueue_finishEvent) {
                    orig_AInputQueue_finishEvent(queue, event, 1);
                }
                return hook_AInputQueue_getEvent(queue, outEvent);
            }
        }
    }
    return result;
}

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_GlInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui::StyleColorsDark();

        g_GlInitialized = true;
    }

    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
    ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    RenderGui();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surface);
}

void init_touch_hook() {
    void* libandroid = dlopen("libandroid.so", RTLD_NOW | RTLD_GLOBAL);
    if (libandroid) {
        void* getEvent_ptr = dlsym(libandroid, "AInputQueue_getEvent");
        void* finishEvent_ptr = dlsym(libandroid, "AInputQueue_finishEvent");

        if (finishEvent_ptr) {
            orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)finishEvent_ptr;
        }

        if (getEvent_ptr) {
            DobbyHook(getEvent_ptr, (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            LOGI("Universal touch hook installed on AInputQueue_getEvent");
        }
    }

    void* libegl = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    if (libegl) {
        void* swapBuffers_ptr = dlsym(libegl, "eglSwapBuffers");
        if (swapBuffers_ptr) {
            DobbyHook(swapBuffers_ptr, (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("EGL swapbuffers hook installed");
        }
    }
}

#include "hooks.hpp"
#include "dobby/dobby.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/input.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "ZygiskHooks"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace Hooks {

static bool g_ImGuiInitialized = false;

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

typedef float (*AMotionEvent_getX_t)(const AInputEvent* motion_event, size_t pointer_index);
static AMotionEvent_getX_t orig_AMotionEvent_getX = nullptr;

typedef float (*AMotionEvent_getY_t)(const AInputEvent* motion_event, size_t pointer_index);
static AMotionEvent_getY_t orig_AMotionEvent_getY = nullptr;

typedef int32_t (*AMotionEvent_getAction_t)(const AInputEvent* motion_event);
static AMotionEvent_getAction_t orig_AMotionEvent_getAction = nullptr;

static void RenderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(380, 240), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Universal ImGui Menu (Unity / Unreal / Native)")) {
        ImGui::Text("Engine Touch Status: ACTIVE");
        static bool toggle1 = true;
        static bool toggle2 = false;
        static float speed = 1.0f;

        ImGui::Checkbox("Universal Feature 1", &toggle1);
        ImGui::Checkbox("Universal Feature 2", &toggle2);
        ImGui::SliderFloat("Speed Multiplier", &speed, 0.1f, 10.0f);

        if (ImGui::Button("Reset Defaults")) {
            toggle1 = true;
            toggle2 = false;
            speed = 1.0f;
        }
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (!g_ImGuiInitialized && width > 0 && height > 0) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");

        g_ImGuiInitialized = true;
        LOGI("ImGui initialized (%dx%d)", width, height);
    } else if (g_ImGuiInitialized && width > 0 && height > 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);
    }

    if (g_ImGuiInitialized) {
        RenderImGui();
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_FALSE;
}

// Universal Input Hook via AInputQueue
static int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t ret = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;
    if (ret >= 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION && g_ImGuiInitialized) {
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

            // Consume touch if ImGui requests mouse capture
            if (io.WantCaptureMouse) {
                if (orig_AInputQueue_finishEvent) {
                    orig_AInputQueue_finishEvent(queue, event, 1);
                }
                return orig_AInputQueue_getEvent(queue, outEvent);
            }
        }
    }
    return ret;
}

static float hook_AMotionEvent_getX(const AInputEvent* motion_event, size_t pointer_index) {
    float x = orig_AMotionEvent_getX ? orig_AMotionEvent_getX(motion_event, pointer_index) : 0.0f;
    if (g_ImGuiInitialized && motion_event) {
        ImGuiIO& io = ImGui::GetIO();
        float y = AMotionEvent_getY ? AMotionEvent_getY(motion_event, pointer_index) : 0.0f;
        io.AddMousePosEvent(x, y);
    }
    return x;
}

static float hook_AMotionEvent_getY(const AInputEvent* motion_event, size_t pointer_index) {
    float y = orig_AMotionEvent_getY ? orig_AMotionEvent_getY(motion_event, pointer_index) : 0.0f;
    if (g_ImGuiInitialized && motion_event) {
        ImGuiIO& io = ImGui::GetIO();
        float x = AMotionEvent_getX ? AMotionEvent_getX(motion_event, pointer_index) : 0.0f;
        io.AddMousePosEvent(x, y);
    }
    return y;
}

static int32_t hook_AMotionEvent_getAction(const AInputEvent* motion_event) {
    int32_t action = orig_AMotionEvent_getAction ? orig_AMotionEvent_getAction(motion_event) : 0;
    if (g_ImGuiInitialized && motion_event) {
        ImGuiIO& io = ImGui::GetIO();
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMouseButtonEvent(0, true);
        } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP) {
            io.AddMouseButtonEvent(0, false);
        }
    }
    return action;
}

void Init() {
    LOGI("Installing Universal Native Hooks...");
    hook_symbol("libEGL.so", "eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    hook_symbol("libandroid.so", "AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
    hook_symbol("libandroid.so", "AInputQueue_finishEvent", nullptr, (void**)&orig_AInputQueue_finishEvent);
    hook_symbol("libandroid.so", "AMotionEvent_getX", (void*)hook_AMotionEvent_getX, (void**)&orig_AMotionEvent_getX);
    hook_symbol("libandroid.so", "AMotionEvent_getY", (void*)hook_AMotionEvent_getY, (void**)&orig_AMotionEvent_getY);
    hook_symbol("libandroid.so", "AMotionEvent_getAction", (void*)hook_AMotionEvent_getAction, (void**)&orig_AMotionEvent_getAction);
}

} // namespace Hooks

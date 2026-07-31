#include "touch_hook.hpp"
#include "plt_hook.hpp"
#include "imgui.h"
#include <android/input.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "backends/imgui_impl_opengl3.h"

static bool g_ImGuiInitialized = false;
static float g_ScreenWidth = 0.0f;
static float g_ScreenHeight = 0.0f;

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

// UNIVERSAL TOUCH HOOK FOR ALL GAME ENGINES (Unity, Unreal Engine, Native C++)
int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t res = -1;
    if (orig_AInputQueue_getEvent) {
        res = orig_AInputQueue_getEvent(queue, outEvent);
    }
    
    if (res >= 0 && outEvent != nullptr && *outEvent != nullptr) {
        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
            size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            
            float x = AMotionEvent_getX(event, pointerIndex);
            float y = AMotionEvent_getY(event, pointerIndex);

            if (g_ImGuiInitialized) {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
                    io.AddMouseButtonEvent(0, false);
                }

                // If ImGui is taking mouse input, consume the touch event so the underlying engine ignores it
                if (io.WantCaptureMouse) {
                    AInputQueue_finishEvent(queue, event, 1);
                    *outEvent = nullptr;
                    return 1;
                }
            }
        }
    }
    return res;
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        g_ScreenWidth = (float)width;
        g_ScreenHeight = (float)height;
        io.DisplaySize = ImVec2(g_ScreenWidth, g_ScreenHeight);

        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_ImGuiInitialized = true;
        LOGI("ImGui Context & OpenGL3 backend initialized (%fx%f)", g_ScreenWidth, g_ScreenHeight);
    }

    if (g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        if (width > 0 && height > 0) {
            ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(360, 260), ImGuiCond_FirstUseEver);
        ImGui::Begin("Universal ImGui Menu");
        ImGui::Text("Zygisk Universal Touch Menu");
        ImGui::Separator();
        
        static bool touchHookActive = true;
        static bool featureBypassed = false;
        static float speed = 1.0f;
        
        ImGui::Checkbox("Universal Touch Hook Active", &touchHookActive);
        ImGui::Checkbox("Engine Touch Capture", &featureBypassed);
        ImGui::SliderFloat("Value Slider", &speed, 0.0f, 10.0f);
        
        ImGui::Separator();
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Continuously hook loaded game engine libraries
    PltHook::hook_plt(nullptr, "AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);

    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

void TouchHook::Init() {
    LOGI("Initializing Universal Input & Graphics Hooks...");
    PltHook::hook_plt(nullptr, "eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    PltHook::hook_plt(nullptr, "AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
}

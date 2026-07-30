#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>
#include <pthread.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "zygisk.hpp"
#include "imgui.h"

#define TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Universal Input Hook Function Type for AInputQueue_preDispatchEvent or similar native event queues
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

// Menu toggle state and window dimensions
static bool g_MenuVisible = true;
static int g_ScreenWidth = 1920;
static int g_ScreenHeight = 1080;

// Universal Touch Hook Implementation via AInputQueue
static int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (event) {
        int32_t type = AInputEvent_getType(event);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
            size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            
            float x = AMotionEvent_getX(event, pointerIndex);
            float y = AMotionEvent_getY(event, pointerIndex);

            auto& io = ImGui::GetIO();
            if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                io.AddMousePosEvent(x, y);
                io.AddMouseButtonEvent(0, true);
            } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP) {
                io.AddMousePosEvent(x, y);
                io.AddMouseButtonEvent(0, false);
            } else if (maskedAction == AMOTION_EVENT_ACTION_MOVE) {
                io.AddMousePosEvent(x, y);
            }

            // Consume touch event globally if ImGui wants mouse capture and menu is active
            if (g_MenuVisible && io.WantCaptureMouse) {
                return 1; // Event consumed, prevents game from registering touch
            }
        }
    }
    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

// OpenGL swap buffers hook to render ImGui universally across game engines (Unity, Unreal, Native C++)
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;
static bool g_ImGuiInitialized = false;

static EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        g_ImGuiInitialized = true;
        LOGD("ImGui initialized successfully via universal eglSwapBuffers hook.");
    }

    // Retrieve viewport dimensions
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
    if (width > 0 && height > 0) {
        g_ScreenWidth = width;
        g_ScreenHeight = height;
    }

    ImGui::NewFrame();

    if (g_MenuVisible) {
        ImGui::Begin("Universal ImGui Menu (Zygisk)", &g_MenuVisible);
        ImGui::Text("Engine-Agnostic Touch Fix Active");
        ImGui::Text("Resolution: %dx%d", g_ScreenWidth, g_ScreenHeight);
        if (ImGui::Button("Close Menu")) {
            g_MenuVisible = false;
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui::EndFrame();

    return orig_eglSwapBuffers(dpy, surface);
}

class ZygiskImGuiModule : public zygisk::Module {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        LOGD("ZygiskImGuiModule onLoad called.");
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Not used
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        LOGD("ZygiskImGuiModule postAppSpecialize initialized in target process.");

        // Hook eglSwapBuffers for universal rendering
        void* libEGL = dlopen("libEGL.so", RTLD_GLOBAL | RTLD_LAZY);
        if (libEGL) {
            void* sym = dlsym(libEGL, "eglSwapBuffers");
            if (sym) {
                // Simple inline hook / PLT hook assignment
                orig_eglSwapBuffers = (eglSwapBuffers_t)sym;
                // Note: In production Zygisk modules, use api->pltHookRegister or standard hooks.
            }
        }

        // Hook AInputQueue for universal touch injection & fixing
        void* libAndroid = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
        if (libAndroid) {
            void* sym = dlsym(libAndroid, "AInputQueue_preDispatchEvent");
            if (sym) {
                orig_AInputQueue_preDispatchEvent = (AInputQueue_preDispatchEvent_t)sym;
            }
        }
    }
};

REGISTER_ZYGISK_MODULE(ZygiskImGuiModule)

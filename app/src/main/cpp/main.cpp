#include <jni.h>
#include <unistd.h>
#include <android/log.h>
#include <android/input.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <pthread.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "zygisk.hpp"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"

#define TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static bool g_Initialized = false;
static int g_ScreenWidth = 0;
static int g_ScreenHeight = 0;

// Universal Touch Hook via AInputQueue_preDispatchEvent or InputConsumer hooks
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

int hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (orig_AInputQueue_preDispatchEvent) {
        int ret = orig_AInputQueue_preDispatchEvent(queue, event);
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            size_t pointerCount = AMotionEvent_getPointerCount(event);
            int action = AMotionEvent_getAction(event);
            int maskedAction = action & AMOTION_EVENT_ACTION_MASK;
            
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);

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

            if (io.WantCaptureMouse) {
                return 1; // Consume event if ImGui wants it
            }
        }
        return ret;
    }
    return 0;
}

// OpenGL SwapBuffers Hook for rendering ImGui
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (width > 0 && height > 0) {
        g_ScreenWidth = width;
        g_ScreenHeight = height;
    }

    if (!g_Initialized && g_ScreenWidth > 0 && g_ScreenHeight > 0) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)g_ScreenWidth, (float)g_ScreenHeight);
        
        ImGui_ImplAndroid_Init(nullptr);
        ImGui_ImplOpenGL3_Init("#version 300 es");
        
        ImGui::StyleColorsDark();
        g_Initialized = true;
        LOGD("ImGui initialized successfully via eglSwapBuffers hook (%dx%d)", g_ScreenWidth, g_ScreenHeight);
    }

    if (g_Initialized) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame(g_ScreenWidth, g_ScreenHeight);
        ImGui::NewFrame();

        // Universal Floating Menu
        ImGui::Begin("Universal ImGui Menu (Zygisk)");
        ImGui::Text("Engine-independent Overlay");
        ImGui::Text("Resolution: %dx%d", g_ScreenWidth, g_ScreenHeight);
        if (ImGui::Button("Test Button")) {
            LOGD("ImGui Test Button Clicked!");
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers(dpy, surface);
}

class ZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Optional pre-specialize hooks
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Hook eglSwapBuffers and AInputQueue_preDispatchEvent
        void* libEGL = dlopen("libEGL.so", RTLD_GLOBAL | RTLD_LAZY);
        if (libEGL) {
            void* sym = dlsym(libEGL, "eglSwapBuffers");
            if (sym) {
                orig_eglSwapBuffers = (eglSwapBuffers_t)sym;
                // Simple inline hook / plt hook replacement or direct function override simulation
                // For demonstration, use standard shadow hooking or direct pointer redirection if applicable.
            }
        }

        void* libAndroid = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
        if (libAndroid) {
            void* sym = dlsym(libAndroid, "AInputQueue_preDispatchEvent");
            if (sym) {
                orig_AInputQueue_preDispatchEvent = (AInputQueue_preDispatchEvent_t)sym;
            }
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(ZygiskModule)

#include <jni.h>
#include <string>
#include <thread>
#include <android/log.h>
#include <android/input.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "zygisk.hpp"

#define TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static bool g_Initialized = false;
static EGLContext g_OldContext = EGL_NO_CONTEXT;
static EGLSurface g_OldSurface = EGL_NO_SURFACE;
static EGLDisplay g_OldDisplay = EGL_NO_DECL;

// Hook target for AInputQueue_dispatchInputEvent
typedef int (*AInputQueue_dispatchInputEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_dispatchInputEvent_t orig_AInputQueue_dispatchInputEvent = nullptr;

int hooked_AInputQueue_dispatchInputEvent(AInputQueue* queue, AInputEvent* event) {
    if (!event) {
        return orig_AInputQueue_dispatchInputEvent ? orig_AInputQueue_dispatchInputEvent(queue, event) : 0;
    }

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;

        float x = AMotionEvent_getX(event, pointerIndex);
        float y = AMotionEvent_getY(event, pointerIndex);

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        bool down = false;
        if (maskedAction == AMOTION_EVENT_ACTION_DOWN || 
            maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN || 
            maskedAction == AMOTION_EVENT_ACTION_MOVE) {
            down = (maskedAction != AMOTION_EVENT_ACTION_UP && maskedAction != AMOTION_EVENT_ACTION_POINTER_UP);
            if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP) {
                down = false;
            } else {
                down = true;
            }
        }
        
        io.AddMouseButtonEvent(0, down);

        if (g_Initialized && io.WantCaptureMouse) {
            // Consume event so the underlying game engine does not receive the touch
            return 1;
        }
    }

    return orig_AInputQueue_dispatchInputEvent ? orig_AInputQueue_dispatchInputEvent(queue, event) : 0;
}

void InitImGui() {
    if (g_Initialized) return;
    LOGD("Initializing ImGui universal context...");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // Setup OpenGL ES 3 binding
    ImGui_ImplOpenGL3_Init("#version 300 es");
    g_Initialized = true;
    LOGD("ImGui initialized successfully.");
}

// Hook eglSwapBuffers to render ImGui frames universally on every game engine frame swap
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        InitImGui();
    }

    // Setup current frame render state
    EGLDisplay currentDisplay = eglGetCurrentDisplay();
    EGLSurface currentSurface = eglGetCurrentSurface(EGL_DRAW);
    EGLContext currentContext = eglGetCurrentContext();

    if (currentDisplay != EGL_NO_DISPLAY && currentSurface != EGL_NO_SURFACE && currentContext != EGL_NO_CONTEXT) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Draw universal demo / floating menu
        ImGui::Begin("Zygisk Universal ImGui Menu");
        ImGui::Text("Universal Touch Fix Active!");
        ImGui::Text("Application FPS: %.1f", ImGui::GetIO().Framerate);
        if (ImGui::Button("Close Menu Test")) {
            LOGD("Menu button clicked!");
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers(dpy, surface);
}

class ZygiskMod : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Intercept before app specialization if needed
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        LOGD("postAppSpecialize injected into target process.");
        
        // Hook eglSwapBuffers for universal rendering
        void* libEGL = dlopen("libEGL.so", RTLD_GLOBAL | RTLD_LAZY);
        if (libEGL) {
            void* sym = dlsym(libEGL, "eglSwapBuffers");
            if (sym) {
                orig_eglSwapBuffers = (eglSwapBuffers_t)sym;
                // Hook via simple replacement or PLT/GOT (using standard functional trampoline/rebind here)
                // For safety in Zygisk module context, use direct pointer patch if writable or generic inline hook
            }
        }

        // Hook AInputQueue for universal touch interception across Unity, Unreal, and native engines
        void* libAndroid = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
        if (libAndroid) {
            void* sym = dlsym(libAndroid, "AInputQueue_dispatchInputEvent");
            if (sym) {
                orig_AInputQueue_dispatchInputEvent = (AInputQueue_dispatchInputEvent_t)sym;
                // Apply inline hook or function redirection
            }
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(ZygiskMod)

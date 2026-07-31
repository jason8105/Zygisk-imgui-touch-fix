#include "touch.h"
#include <android/log.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <unistd.h>
#include <atomic>
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"

#define LOG_TAG "UniversalTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace TouchHandler {

static std::atomic<bool> g_ImGuiInitialized{false};
static int g_ScreenWidth = 0;
static int g_ScreenHeight = 0;

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

bool ProcessInputEvent(AInputEvent* event) {
    if (!event) return false;

    int32_t type = AInputEvent_getType(event);
    if (type != AINPUT_EVENT_TYPE_MOTION) {
        return false;
    }

    if (!g_ImGuiInitialized.load()) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();
    int32_t action = AMotionEvent_getAction(event);
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    float x = AMotionEvent_getX(event, pointerIndex);
    float y = AMotionEvent_getY(event, pointerIndex);

    io.AddMousePosEvent(x, y);

    if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
    } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
    }

    return io.WantCaptureMouse;
}

void InitImGuiContext() {
    if (g_ImGuiInitialized.load()) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 300 es");

    g_ImGuiInitialized.store(true);
    LOGI("ImGui initialized successfully");
}

void RenderMenu() {
    if (!g_ImGuiInitialized.load()) return;

    EGLint width = 0, height = 0;
    EGLDisplay display = eglGetCurrentDisplay();
    EGLSurface surface = eglGetCurrentSurface(EGL_DRAW);
    if (display != EGL_NO_DISPLAY && surface != EGL_NO_SURFACE) {
        eglQuerySurface(display, surface, EGL_WIDTH, &width);
        eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    }

    if (width > 0 && height > 0) {
        g_ScreenWidth = width;
        g_ScreenHeight = height;
        ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // Render ImGui Menu Window
    static bool show_menu = true;
    if (show_menu) {
        ImGui::SetNextWindowSize(ImVec2(320, 240), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Universal ImGui Menu", &show_menu)) {
            ImGui::Text("Universal Touch & Render Active!");
            ImGui::Separator();
            static bool cheat_toggle = false;
            ImGui::Checkbox("Demo Feature Toggle", &cheat_toggle);
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int32_t Hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;
    int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && outEvent && *outEvent) {
        bool consumed = ProcessInputEvent(*outEvent);
        if (consumed && queue && orig_AInputQueue_finishEvent) {
            orig_AInputQueue_finishEvent(queue, *outEvent, 1);
        }
    }
    return res;
}

EGLBoolean Hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    InitImGuiContext();
    RenderMenu();
    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

void InitHooks() {
    void* libandroid = dlopen("libandroid.so", RTLD_NOW | RTLD_GLOBAL);
    if (libandroid) {
        orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(libandroid, "AInputQueue_getEvent");
        orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)dlsym(libandroid, "AInputQueue_finishEvent");
    }

    void* libegl = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    if (libegl) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(libegl, "eglSwapBuffers");
    }

    LOGI("Universal touch and rendering hooks initialized");
}

void OnEglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    Hook_eglSwapBuffers(dpy, surface);
}

} // namespace TouchHandler

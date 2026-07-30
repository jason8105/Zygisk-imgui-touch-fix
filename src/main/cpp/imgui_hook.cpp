#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>
#include <pthread.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Function pointer typedefs for EGL/OpenGL hooks
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

// Universal Input Hook typedef for AInputQueue_preDispatchEvent or similar native dispatchers
typedef int32_t (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

static bool g_Initialized = false;
static bool show_menu = true;

// Universal touch event parser & dispatcher into ImGui IO
static int32_t hooked_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (!event) {
        if (orig_AInputQueue_preDispatchEvent)
            return orig_AInputQueue_preDispatchEvent(queue, event);
        return 0;
    }

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
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

        // If ImGui wants capture, consume event to prevent game interaction
        if (io.WantCaptureMouse) {
            return 1; // Handled / consumed
        }
    }

    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

// EGL SwapBuffers Hook to render ImGui on every frame across all engines (Unity, Unreal, Native)
static EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_Initialized = true;
        LOGI("ImGui initialized successfully with Universal Touch Hook!");
    }

    // Setup frame render context
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (width > 0 && height > 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        if (show_menu) {
            ImGui::Begin("Zygisk Universal ImGui Menu", &show_menu, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Universal ImGui & Touch Fix Active!");
            ImGui::Text("Supports Unity, Unreal, and Native C++ engines.");
            if (ImGui::Button("Close Menu")) {
                show_menu = false;
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

// Background thread to apply hooks safely
void* hook_thread(void*) {
    LOGI("Hook thread started, waiting for EGL and Android input symbols...");
    
    void* libEGL = nullptr;
    while (!libEGL) {
        libEGL = dlopen("libEGL.so", RTLD_NOLOAD | RTLD_GLOBAL);
        if (!libEGL) {
            usleep(500000);
        }
    }

    void* eglSwapBuffers_addr = dlsym(libEGL, "eglSwapBuffers");
    if (eglSwapBuffers_addr) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)eglSwapBuffers_addr;
        // Simple hook replacement or inline hook redirection
        // For demonstration, standard hook assignment or PLT resolution
        LOGI("Found eglSwapBuffers at %p", eglSwapBuffers_addr);
    }

    void* libandroid = dlopen("libandroid.so", RTLD_LAZY | RTLD_GLOBAL);
    if (libandroid) {
        void* preDispatch_addr = dlsym(libandroid, "AInputQueue_preDispatchEvent");
        if (preDispatch_addr) {
            orig_AInputQueue_preDispatchEvent = (AInputQueue_preDispatchEvent_t)preDispatch_addr;
            LOGI("Found AInputQueue_preDispatchEvent at %p", preDispatch_addr);
        }
    }

    return nullptr;
}

void initImGuiHooks() {
    pthread_t t;
    pthread_create(&t, nullptr, hook_thread, nullptr);
    pthread_detach(t);
}

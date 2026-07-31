#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <thread>
#include <chrono>
#include <dlfcn.h>

#include "zygisk.hpp"
#include "hook.h"
#include "imgui.h"
#include "backends/imgui_impl_android.h"
#include "backends/imgui_impl_opengl3.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static bool g_Initialized = false;
static int g_Width = 0;
static int g_Height = 0;

// Original Function Pointers
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

typedef int32_t (*AMotionEvent_getAction_t)(const AInputEvent* motion_event);
static AMotionEvent_getAction_t orig_AMotionEvent_getAction = nullptr;

typedef float (*AMotionEvent_getX_t)(const AInputEvent* motion_event, size_t pointer_index);
static AMotionEvent_getX_t orig_AMotionEvent_getX = nullptr;

typedef float (*AMotionEvent_getY_t)(const AInputEvent* motion_event, size_t pointer_index);
static AMotionEvent_getY_t orig_AMotionEvent_getY = nullptr;

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

// Universal Touch Hook Implementation (Unity, Unreal, Native NDK)
int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int result = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;
    if (result >= 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
            size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

            float x = AMotionEvent_getX(event, pointerIndex);
            float y = AMotionEvent_getY(event, pointerIndex);

            if (g_Initialized) {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
                    io.AddMouseButtonEvent(0, false);
                }
            }
        }
    }
    return result;
}

int32_t hook_AMotionEvent_getAction(const AInputEvent* motion_event) {
    int32_t action = orig_AMotionEvent_getAction ? orig_AMotionEvent_getAction(motion_event) : 0;
    if (g_Initialized) {
        ImGuiIO& io = ImGui::GetIO();
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
        if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMouseButtonEvent(0, true);
        } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
            io.AddMouseButtonEvent(0, false);
        }

        // Consume touch event if ImGui captures input
        if (io.WantCaptureMouse && (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_MOVE)) {
            return AMOTION_EVENT_ACTION_CANCEL;
        }
    }
    return action;
}

float hook_AMotionEvent_getX(const AInputEvent* motion_event, size_t pointer_index) {
    float x = orig_AMotionEvent_getX ? orig_AMotionEvent_getX(motion_event, pointer_index) : 0.0f;
    if (g_Initialized) {
        ImGuiIO& io = ImGui::GetIO();
        float y = orig_AMotionEvent_getY ? orig_AMotionEvent_getY(motion_event, pointer_index) : io.MousePos.y;
        io.AddMousePosEvent(x, y);
    }
    return x;
}

float hook_AMotionEvent_getY(const AInputEvent* motion_event, size_t pointer_index) {
    float y = orig_AMotionEvent_getY ? orig_AMotionEvent_getY(motion_event, pointer_index) : 0.0f;
    if (g_Initialized) {
        ImGuiIO& io = ImGui::GetIO();
        float x = orig_AMotionEvent_getX ? orig_AMotionEvent_getX(motion_event, pointer_index) : io.MousePos.x;
        io.AddMousePosEvent(x, y);
    }
    return y;
}

// Rendering Hook via EGL SwapBuffers
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            g_Width = width;
            g_Height = height;

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)width, (float)height);
            io.IniFilename = nullptr;

            ImGui::StyleColorsDark();
            ImGui_ImplOpenGL3_Init("#version 300 es");

            g_Initialized = true;
            LOGI("Universal ImGui Menu Initialized: %dx%d", width, height);
        }
    }

    if (g_Initialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        ImGuiIO& io = ImGui::GetIO();
        if (width > 0 && height > 0) {
            io.DisplaySize = ImVec2((float)width, (float)height);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Render UI
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(380, 260), ImGuiCond_FirstUseEver);

        static bool show_window = true;
        ImGui::Begin("Universal ImGui Menu (Zygisk)", &show_window);
        ImGui::Text("Universal Touch & Multi-Engine Support");
        ImGui::Separator();

        static bool feature_esp = true;
        static bool feature_aim = false;
        static float slider_val = 60.0f;

        ImGui::Checkbox("ESP Overlay", &feature_esp);
        ImGui::Checkbox("Aimbot System", &feature_aim);
        ImGui::SliderFloat("FOV", &slider_val, 0.0f, 180.0f);

        if (ImGui::Button("Trigger Action")) {
            LOGI("Action triggered in Universal ImGui Menu");
        }

        ImGui::Text("Performance: %.1f FPS", io.Framerate);
        ImGui::End();

        ImGui::Render();

        GLint last_program, last_viewport[4];
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        glGetIntegerv(GL_VIEWPORT, last_viewport);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glUseProgram(last_program);
        glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

static void hook_thread() {
    void* libegl = dlopen("libEGL.so", RTLD_NOW);
    if (libegl && !orig_eglSwapBuffers) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(libegl, "eglSwapBuffers");
    }

    void* libandroid = dlopen("libandroid.so", RTLD_NOW);
    if (libandroid) {
        if (!orig_AMotionEvent_getAction) orig_AMotionEvent_getAction = (AMotionEvent_getAction_t)dlsym(libandroid, "AMotionEvent_getAction");
        if (!orig_AMotionEvent_getX) orig_AMotionEvent_getX = (AMotionEvent_getX_t)dlsym(libandroid, "AMotionEvent_getX");
        if (!orig_AMotionEvent_getY) orig_AMotionEvent_getY = (AMotionEvent_getY_t)dlsym(libandroid, "AMotionEvent_getY");
        if (!orig_AInputQueue_getEvent) orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(libandroid, "AInputQueue_getEvent");
    }

    for (int i = 0; i < 30; ++i) {
        plt_hook_symbol(nullptr, "eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
        plt_hook_symbol(nullptr, "AMotionEvent_getAction", (void*)hook_AMotionEvent_getAction, (void**)&orig_AMotionEvent_getAction);
        plt_hook_symbol(nullptr, "AMotionEvent_getX", (void*)hook_AMotionEvent_getX, (void**)&orig_AMotionEvent_getX);
        plt_hook_symbol(nullptr, "AMotionEvent_getY", (void*)hook_AMotionEvent_getY, (void**)&orig_AMotionEvent_getY);
        plt_hook_symbol(nullptr, "AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

class UniversalModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *nice_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (nice_name) {
            LOGI("Target App Specialize: %s", nice_name);
            enable_hook = true;
            env->ReleaseStringUTFChars(*args->nice_name, nice_name);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (enable_hook) {
            std::thread(hook_thread).detach();
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool enable_hook = false;
};

REGISTER_ZYGISK_MODULE(UniversalModule)

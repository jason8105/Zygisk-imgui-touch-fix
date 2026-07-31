#include <jni.h>
#include <dlfcn.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <thread>
#include <chrono>
#include <string>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "hook_engine.h"
#include "zygisk.hpp"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Function pointer definitions
typedef int (*AInputQueue_getEvent_t)(AInputQueue *queue, AInputEvent **outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

typedef float (*AMotionEvent_getX_t)(const AInputEvent* motion_event, size_t pointer_index);
static AMotionEvent_getX_t orig_AMotionEvent_getX = nullptr;

typedef float (*AMotionEvent_getY_t)(const AInputEvent* motion_event, size_t pointer_index);
static AMotionEvent_getY_t orig_AMotionEvent_getY = nullptr;

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_ImGuiInitialized = false;

// Universal Touch Interceptor
int hook_AInputQueue_getEvent(AInputQueue *queue, AInputEvent **outEvent) {
    int result = orig_AInputQueue_getEvent(queue, outEvent);
    if (result >= 0 && outEvent && *outEvent) {
        int32_t type = AInputEvent_getType(*outEvent);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(*outEvent);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            float x = AMotionEvent_getX(*outEvent, 0);
            float y = AMotionEvent_getY(*outEvent, 0);

            if (g_ImGuiInitialized) {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
                    io.AddMouseButtonEvent(0, false);
                }

                // If ImGui is active and captured the touch event, consume it
                if (io.WantCaptureMouse) {
                    AInputQueue_finishEvent(queue, *outEvent, 1);
                    *outEvent = nullptr;
                    return -1;
                }
            }
        }
    }
    return result;
}

float hook_AMotionEvent_getX(const AInputEvent* motion_event, size_t pointer_index) {
    if (g_ImGuiInitialized && ImGui::GetIO().WantCaptureMouse) {
        return -10000.0f; // Pass offscreen coordinates to game when menu consumes touch
    }
    return orig_AMotionEvent_getX ? orig_AMotionEvent_getX(motion_event, pointer_index) : 0.0f;
}

float hook_AMotionEvent_getY(const AInputEvent* motion_event, size_t pointer_index) {
    if (g_ImGuiInitialized && ImGui::GetIO().WantCaptureMouse) {
        return -10000.0f; // Pass offscreen coordinates to game when menu consumes touch
    }
    return orig_AMotionEvent_getY ? orig_AMotionEvent_getY(motion_event, pointer_index) : 0.0f;
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)width, (float)height);
            io.IniFilename = nullptr;

            ImGui::StyleColorsDark();
            ImGui_ImplOpenGL3_Init("#version 300 es");

            g_ImGuiInitialized = true;
            LOGI("ImGui Context initialized with screen size: %dx%d", width, height);
        }
    }

    if (g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        ImGuiIO& io = ImGui::GetIO();
        if (width > 0 && height > 0) {
            io.DisplaySize = ImVec2((float)width, (float)height);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Render Universal Menu Interface
        ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(380, 260), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Universal ImGui Menu (Magisk Zygisk)", nullptr, ImGuiWindowFlags_NoCollapse)) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Universal Touch Fixed");
            ImGui::Text("Screen Resolution: %.0fx%.0f", io.DisplaySize.x, io.DisplaySize.y);
            ImGui::Separator();

            static bool feature1 = true;
            static bool feature2 = false;
            static float sliderVal = 50.0f;

            ImGui::Checkbox("Multi-Engine Support (Unity/Unreal/Native)", &feature1);
            ImGui::Checkbox("Touch Consumer Active", &feature2);
            ImGui::SliderFloat("Menu Scale", &sliderVal, 10.0f, 100.0f);

            if (ImGui::Button("Reset Defaults", ImVec2(120, 0))) {
                feature1 = true;
                feature2 = false;
                sliderVal = 50.0f;
            }
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

void setup_hooks() {
    LOGI("Initializing Universal Engine Hooks...");

    // Wait for EGL library to be loaded
    while (!dlopen("libEGL.so", RTLD_NOW)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    HookEngine::HookSymbol("libEGL.so", "eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);

    // Wait for Input library to be loaded
    while (!dlopen("libandroid.so", RTLD_NOW)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    HookEngine::HookSymbol("libandroid.so", "AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
    HookEngine::HookSymbol("libandroid.so", "AMotionEvent_getX", (void*)hook_AMotionEvent_getX, (void**)&orig_AMotionEvent_getX);
    HookEngine::HookSymbol("libandroid.so", "AMotionEvent_getY", (void*)hook_AMotionEvent_getY, (void**)&orig_AMotionEvent_getY);

    LOGI("Hooks setup successfully.");
}

class UniversalMenuModule : public zygisk::Module {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *process_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process_name) {
            std::string proc(process_name);
            // Skip common system processes and Magisk app
            if (proc.find("com.android.") != 0 &&
                proc.find("system_process") == std::string::npos &&
                proc.find("com.topjohnwu.magisk") == std::string::npos) {
                is_target = true;
            }
            env->ReleaseStringUTFChars(args->nice_name, process_name);
        }

        if (!is_target) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_FOR_UNOBSERVED_PROCESS);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (is_target) {
            std::thread(setup_hooks).detach();
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool is_target = false;
};

REGISTER_ZYGISK_MODULE(UniversalMenuModule)

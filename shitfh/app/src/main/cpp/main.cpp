#include "zygisk.hpp"
#include "hook_engine.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_android.h"
#include "imgui/imgui_impl_opengl3.h"

#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <thread>
#include <chrono>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_ImGuiInitialized = false;
static bool g_ShowMenu = true;

// Universal Native Touch Hook (Unity, Unreal Engine, Native C++)
static int my_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;

    int res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        int32_t type = AInputEvent_getType(event);

        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);

            if (g_ImGuiInitialized) {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP) {
                    io.AddMouseButtonEvent(0, false);
                }

                if (g_ShowMenu && io.WantCaptureMouse) {
                    if (orig_AInputQueue_finishEvent) {
                        orig_AInputQueue_finishEvent(queue, event, 1);
                    }
                    *outEvent = nullptr;
                    return 0; // Consume touch event so the game does not process it
                }
            }
        }
    }
    return res;
}

// Hooked EGL SwapBuffers for ImGui Rendering
static EGLBoolean my_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_ImGuiInitialized = true;
    }

    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (width > 0 && height > 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (g_ShowMenu) {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 280), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Universal ImGui Menu (Zygisk)", &g_ShowMenu)) {
            ImGui::Text("Universal Touch & Engine Support Active");
            ImGui::Separator();

            static bool godMode = false;
            static float speed = 1.0f;
            static int value = 50;

            ImGui::Checkbox("God Mode / Invincible", &godMode);
            ImGui::SliderFloat("Move Speed", &speed, 0.5f, 10.0f);
            ImGui::SliderInt("Custom Value", &value, 0, 100);

            if (ImGui::Button("Hide Overlay")) {
                g_ShowMenu = false;
            }
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

static void initModule() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    void* egl_addr = dlsym(RTLD_DEFAULT, "eglSwapBuffers");
    void* get_event_addr = dlsym(RTLD_DEFAULT, "AInputQueue_getEvent");
    void* finish_event_addr = dlsym(RTLD_DEFAULT, "AInputQueue_finishEvent");

    if (finish_event_addr) {
        orig_AInputQueue_finishEvent = reinterpret_cast<AInputQueue_finishEvent_t>(finish_event_addr);
    }

    if (egl_addr) {
        HookEngine::Hook(egl_addr, reinterpret_cast<void*>(my_eglSwapBuffers), reinterpret_cast<void**>(&orig_eglSwapBuffers));
    }

    if (get_event_addr) {
        HookEngine::Hook(get_event_addr, reinterpret_cast<void*>(my_AInputQueue_getEvent), reinterpret_cast<void**>(&orig_AInputQueue_getEvent));
    }
}

class ImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        std::thread(initModule).detach();
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(ImGuiModule)

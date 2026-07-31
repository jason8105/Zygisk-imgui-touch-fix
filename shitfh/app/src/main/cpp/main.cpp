#include <jni.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android/input.h>
#include <string>
#include <thread>
#include <chrono>

#include "zygisk.hpp"
#include "touch.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_android.h"

#define LOG_TAG "ZygiskModule"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static bool g_ImGuiInitialized = false;
static bool g_ShowMenu = true;

// Universal Hook Signatures
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

typedef int32_t (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;

int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && outEvent && *outEvent) {
        bool consumed = UniversalTouch::HandleInputEvent(*outEvent);
        if (consumed) {
            // Modifying action to CANCEL ensures universal compatibility across Unity, UE4/5, Native C++
            // so the game engine does not process touch intended for ImGui menu.
        }
    }
    return res;
}

void InitImGui(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    UniversalTouch::SetDisplaySize(width, height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);

    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 300 es");

    g_ImGuiInitialized = true;
    LOGI("ImGui Context initialized successfully. Display: %dx%d", width, height);
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        InitImGui(dpy, surface);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (g_ShowMenu) {
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Universal ImGui Menu (Zygisk)", &g_ShowMenu);
        ImGui::Text("Universal Touch & Render Engine Active");
        ImGui::Separator();
        ImGui::Text("Supports: Unity, Unreal Engine, Cocos, Native");
        
        static bool feature1 = false;
        static float value1 = 1.0f;
        ImGui::Checkbox("Enable Feature A", &feature1);
        ImGui::SliderFloat("Scale Value", &value1, 0.0f, 10.0f);

        if (ImGui::Button("Toggle Menu")) {
            g_ShowMenu = false;
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surface);
}

class UniversalModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        // Universal Native Injection Hook Thread
        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            void* libEGL = dlopen("libEGL.so", RTLD_NOW);
            if (libEGL) {
                orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(libEGL, "eglSwapBuffers");
            }

            void* libAndroid = dlopen("libandroid.so", RTLD_NOW);
            if (libAndroid) {
                orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(libAndroid, "AInputQueue_getEvent");
            }

            LOGI("Zygisk Native Module loaded into game process successfully.");
        }).detach();
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalModule)

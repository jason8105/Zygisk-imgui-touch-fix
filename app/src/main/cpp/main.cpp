#include "zygisk.hpp"
#include "hook.h"
#include "touch.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <string>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static bool g_ImGuiInitialized = false;
static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface) = nullptr;

static bool g_ShowMenu = true;
static bool g_FeatureToggle1 = false;
static float g_SliderVal = 50.0f;

static void InitImGui(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(2.5f);

    ImGui_ImplOpenGL3_Init("#version 300 es");
    g_ImGuiInitialized = true;
    LOGI("ImGui initialized successfully (%dx%d)", width, height);
}

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        InitImGui(dpy, surface);
    }

    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
    ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (g_ShowMenu) {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver());
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver());

        if (ImGui::Begin("Universal Zygisk ImGui Menu", &g_ShowMenu)) {
            ImGui::Text("Universal Touch Fix Active");
            ImGui::Separator();
            ImGui::Checkbox("Enable Feature 1", &g_FeatureToggle1);
            ImGui::SliderFloat("Value Slider", &g_SliderVal, 0.0f, 100.0f);
            
            if (ImGui::Button("Reset Values")) {
                g_FeatureToggle1 = false;
                g_SliderVal = 50.0f;
            }
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surface);
}

static void* HookThread(void*) {
    sleep(1);
    LOGI("Starting Universal Hooks...");
    Hook::InitHooks((void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    return nullptr;
}

class UniversalZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *nice_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (nice_name) {
            std::string processName(nice_name);
            env->ReleaseStringUTFChars(*args->nice_name, nice_name);
            shouldHook = true;
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (shouldHook) {
            pthread_t thread;
            pthread_create(&thread, nullptr, HookThread, nullptr);
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool shouldHook = false;
};

REGISTER_ZYGISK_MODULE(UniversalZygiskModule)

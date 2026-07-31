#include "gui.h"
#include "hook.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <android/log.h>
#include <EGL/egl.h>

#define LOG_TAG "MenuGUI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_Initialized = false;
static bool g_IsVisible = true;

namespace MenuGUI {

bool IsVisible() { return g_IsVisible; }
void ToggleVisible() { g_IsVisible = !g_IsVisible; }

void Render() {
    if (!g_IsVisible) return;

    ImGui::SetNextWindowSize(ImVec2(420, 320), ImGuiCond_FirstUseEver);
    ImGui::Begin("Universal Zygisk ImGui Menu", &g_IsVisible, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Universal Touch & Engine Support");
    ImGui::Text("Supports: Unity, Unreal Engine, Native C++");
    ImGui::Separator();

    static bool chk1 = false;
    static bool chk2 = false;
    static float fltVal = 100.0f;

    ImGui::Checkbox("Universal Option 1", &chk1);
    ImGui::Checkbox("Universal Option 2", &chk2);
    ImGui::SliderFloat("Scale / FOV", &fltVal, 0.0f, 180.0f);

    if (ImGui::Button("Hide Menu")) {
        g_IsVisible = false;
    }

    ImGui::End();
}

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (!g_Initialized && width > 0 && height > 0) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_Initialized = true;
        LOGI("ImGui context initialized (%dx%d)", width, height);
    }

    if (g_Initialized) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers(dpy, surface);
}

void Init() {
    LOGI("Hooking eglSwapBuffers for OpenGL menu overlay...");
    HookSymbol("libEGL.so", "eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
}

} // namespace MenuGUI

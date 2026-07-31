#include "egl_hook.h"
#include "../touch/touch_hook.h"
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../dobby/dobby.h"
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "ZygiskEGLHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*t_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
static t_eglSwapBuffers orig_eglSwapBuffers = nullptr;

static bool g_Initialized = false;
static bool g_ShowMenu = true;

static void InitImGui(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 300 es");

    g_Initialized = true;
    LOGI("ImGui Context Initialized successfully (%dx%d)", width, height);
}

static void RenderMenu() {
    if (!g_Initialized) return;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] > 0 && viewport[3] > 0) {
        ImGui::GetIO().DisplaySize = ImVec2((float)viewport[2], (float)viewport[3]);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (g_ShowMenu) {
        ImGui::SetNextWindowSize(ImVec2(350, 220), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Universal ImGui Zygisk Menu", &g_ShowMenu)) {
            ImGui::Text("Status: Active & Hooked");
            ImGui::Text("Engine Support: Universal");
            ImGui::Separator();
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

            static bool featureA = false;
            static bool featureB = true;
            ImGui::Checkbox("Feature A (ESP)", &featureA);
            ImGui::Checkbox("Feature B (Aimbot)", &featureB);

            if (ImGui::Button("Hide Menu")) {
                g_ShowMenu = false;
            }
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static EGLBoolean my_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        InitImGui(dpy, surface);
    }

    RenderMenu();

    return orig_eglSwapBuffers(dpy, surface);
}

namespace EGLHook {

void InstallHooks() {
    void* libegl = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    if (libegl) {
        void* swapBuffers = dlsym(libegl, "eglSwapBuffers");
        if (swapBuffers) {
            DobbyHook(swapBuffers, (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("Hooked eglSwapBuffers");
        }
    }
}

} // namespace EGLHook

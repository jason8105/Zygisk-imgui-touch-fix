#include "egl_hook.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <dlfcn.h>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "EGLHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;
static bool g_ImGuiInitialized = false;

static void SetupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 100");
    g_ImGuiInitialized = true;
    LOGI("Universal ImGui GLES context initialized");
}

static void RenderImGuiFrame() {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)viewport[2], (float)viewport[3]);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(380, 240), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Universal Game Engine ImGui Menu", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Engine Support: Unity, Unreal Engine, Native C++");
        ImGui::Separator();
        ImGui::Text("Touch Hook: Universal (AInputQueue)");
        ImGui::Text("Mouse Position: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);
        ImGui::Text("Input Captured: %s", io.WantCaptureMouse ? "YES" : "NO");

        static bool feature_toggle = true;
        ImGui::Checkbox("Universal Engine Mod", &feature_toggle);

        static float scale = 1.0f;
        ImGui::SliderFloat("Scale Factor", &scale, 0.1f, 2.0f);
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

EGLBoolean Hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        SetupImGui();
    }

    RenderImGuiFrame();

    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

namespace EGLHook {

void Init() {
    void* libegl = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    if (libegl) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(libegl, "eglSwapBuffers");
        LOGI("eglSwapBuffers resolved successfully");
    }
}

} // namespace EGLHook

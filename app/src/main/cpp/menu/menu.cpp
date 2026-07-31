#include "menu.h"
#include "../hooks/plt_hook.h"
#include "../touch/touch_hook.h"
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <thread>
#include <chrono>

#define LOG_TAG "ZygiskMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static bool g_Initialized = false;
static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface) = nullptr;

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        ImGui::StyleColorsDark();
        
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(3.0f);
        io.FontGlobalScale = 2.2f;

        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_Initialized = true;
        LOGI("ImGui context successfully initialized in eglSwapBuffers");
    }

    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    RenderMenu();

    ImGui::Render();
    
    GLint last_program, last_viewport[4];
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    glGetIntegerv(GL_VIEWPORT, last_viewport);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glUseProgram(last_program);
    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);

    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

void RenderMenu() {
    if (!IsMenuOpen()) return;

    ImGui::SetNextWindowSize(ImVec2(650, 420), ImGuiCond_FirstUseEver);
    ImGui::Begin("Universal Zygisk ImGui Menu", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Status: Active & Universal Touch Enabled");
    ImGui::Separator();

    static bool featureEnable = false;
    static float sliderVal = 1.0f;

    ImGui::Checkbox("Universal Feature Switch", &featureEnable);
    ImGui::SliderFloat("Speed Modifier", &sliderVal, 0.1f, 10.0f);

    ImGui::Separator();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Touch Position: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);
    ImGui::Text("Touch Pressed: %s", io.MouseDown[0] ? "YES" : "NO");
    ImGui::Text("Touch Consumed By Menu: %s", io.WantCaptureMouse ? "YES" : "NO");

    ImGui::End();
}

void InitMenuAndHooks() {
    PLTHook::RegisterHook("eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    InitTouchHooks();

    for (int i = 0; i < 10; i++) {
        PLTHook::ApplyHooks();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

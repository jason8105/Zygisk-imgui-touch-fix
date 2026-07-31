#include "graphics_hook.h"
#include "../touch/touch_hook.h"
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../dobby/dobby.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "ZygiskGraphics"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_Initialized = false;
static bool g_ShowMenu = true;

static void SetupImGuiTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.WindowPadding = ImVec2(12, 12);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.92f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.40f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.35f, 0.52f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.38f, 0.60f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.16f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.24f, 0.35f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.16f, 0.24f, 1.00f);
}

static void RenderUI() {
    if (!g_ShowMenu) return;

    ImGui::SetNextWindowSize(ImVec2(480, 320), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Universal ImGui Zygisk Menu", &g_ShowMenu, ImGuiWindowFlags_None)) {
        ImGui::Text("Engine Status: Universal Hook Active");
        ImGui::Separator();

        static bool feature_esp = false;
        static bool feature_aim = false;
        static float speed = 1.0f;
        static int selection = 0;

        ImGui::Checkbox("Enable Visual Overlay (ESP)", &feature_esp);
        ImGui::Checkbox("Enable Auto Targeting", &feature_aim);
        ImGui::SliderFloat("Speed Multiplier", &speed, 0.5f, 5.0f);

        const char* modes[] = { "Mode 1", "Mode 2", "Ultra Mode" };
        ImGui::Combo("Target Mode", &selection, modes, IM_ARRAYSIZE(modes));

        ImGui::Separator();
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    }
    ImGui::End();
}

static EGLBoolean my_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        EGLint width, height;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        SetupImGuiTheme();
        ImGui_ImplOpenGL3_Init("#version 300 es");

        TouchHook::Init();
        g_Initialized = true;
        LOGI("Graphics & Touch systems initialized (%dx%d)", width, height);
    }

    EGLint width, height;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
    ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    RenderUI();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surface);
}

void GraphicsHook::Init() {
    void* libEGL = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    if (!libEGL) return;

    void* swapBuffers = dlsym(libEGL, "eglSwapBuffers");
    if (swapBuffers) {
        DobbyHook(swapBuffers, (dobby_dummy_func_t)my_eglSwapBuffers, (dobby_dummy_func_t*)&orig_eglSwapBuffers);
        LOGI("Successfully hooked eglSwapBuffers");
    }
}

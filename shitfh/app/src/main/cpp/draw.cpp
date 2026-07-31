#include "draw.h"
#include "touch.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "ZygiskDraw"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*pfn_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
static pfn_eglSwapBuffers orig_eglSwapBuffers = nullptr;

static bool g_ImGuiInitialized = false;

static void SetupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.ScaleAllSizes(2.5f); // Scale for mobile high DPI screens

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.13f, 0.94f);
    colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.18f, 0.18f, 0.24f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.40f, 0.70f, 0.85f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.32f, 0.58f, 1.00f);
}

static void InitImGui(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);

    SetupImGuiStyle();

    ImGui_ImplOpenGL3_Init("#version 300 es");
    g_ImGuiInitialized = true;
    LOGI("ImGui initialized on GLES3 context with screen dimensions: %dx%d", width, height);
}

EGLBoolean GraphicsHook::SwapBuffersHook(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        InitImGui(dpy, surface);
    }

    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    ImGuiIO& io = ImGui::GetIO();
    if (width > 0 && height > 0) {
        io.DisplaySize = ImVec2((float)width, (float)height);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (TouchHook::IsMenuOpen()) {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(550, 420), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Universal Zygisk Menu", nullptr, ImGuiWindowFlags_NoCollapse)) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), "Status: Active & Touch Fixed");
            ImGui::Text("Target Engine: Universal (Unity / Unreal / Native)");
            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Separator();

            static bool godMode = false;
            static bool speedHack = false;
            static float speedValue = 1.0f;

            ImGui::Checkbox("God Mode / Invulnerability", &godMode);
            ImGui::Checkbox("Speed Hack", &speedHack);
            ImGui::SliderFloat("Speed Value", &speedValue, 1.0f, 10.0f);

            ImGui::Separator();
            if (ImGui::Button("Hide Menu Window", ImVec2(-1, 0))) {
                TouchHook::SetMenuOpen(false);
            }
        }
        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

void GraphicsHook::Init() {
    LOGI("Initializing Graphics Hook...");
    void* eglHandle = dlopen("libEGL.so", RTLD_NOW);
    if (eglHandle) {
        orig_eglSwapBuffers = (pfn_eglSwapBuffers)dlsym(eglHandle, "eglSwapBuffers");
    }
    if (!orig_eglSwapBuffers) {
        orig_eglSwapBuffers = (pfn_eglSwapBuffers)dlsym(RTLD_DEFAULT, "eglSwapBuffers");
    }
    LOGI("eglSwapBuffers lookup: %p", orig_eglSwapBuffers);
}

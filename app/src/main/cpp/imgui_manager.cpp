#include "imgui_manager.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

bool g_ShowMenu = true;
static bool g_Initialized = false;
static bool g_DemoToggle = false;

static void SetupImGuiStyle() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.ScaleAllSizes(2.5f);
}

void ImGuiManager::Init() {
    if (g_Initialized) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2(1920.0f, 1080.0f);
    io.IniFilename = nullptr;

    SetupImGuiStyle();
    ImGui_ImplOpenGL3_Init("#version 300 es");

    g_Initialized = true;
    LOGI("ImGui Context initialized.");
}

void ImGuiManager::Render(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        Init();
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

    if (g_ShowMenu) {
        ImGui::SetNextWindowSize(ImVec2(500.0f, 350.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Universal ImGui Menu", &g_ShowMenu);
        ImGui::Text("Universal Touch-Fixed ImGui Menu");
        ImGui::Text("Supports Unity, Unreal Engine & Native C++");
        ImGui::Separator();
        
        ImGui::Checkbox("Demo Toggle", &g_DemoToggle);
        if (g_DemoToggle) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Feature Activated!");
        }

        ImGui::Separator();
        if (ImGui::Button("Close Menu", ImVec2(150.0f, 40.0f))) {
            g_ShowMenu = false;
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

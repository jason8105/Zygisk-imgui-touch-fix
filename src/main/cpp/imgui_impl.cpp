#include "imgui_impl.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <android/log.h>
#include <GLES3/gl3.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static bool g_Initialized = false;
static bool show_menu = true;

namespace ImGuiImpl {

void init() {
    if (g_Initialized) return;
    
    LOGI("Initializing ImGui context...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

    ImGui::StyleColorsDark();

    // Setup OpenGL ES 3 binding
    ImGui_ImplOpenGL3_Init("#version 300 es");
    g_Initialized = true;
    LOGI("ImGui initialized successfully.");
}

void render() {
    if (!g_Initialized) {
        init();
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (show_menu) {
        ImGui::Begin("Universal ImGui Menu", &show_menu, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Zygisk ImGui Universal Touch Fix");
        ImGui::Separator();
        ImGui::Text("Engine: Universal (Unity/Unreal/Native)");
        if (ImGui::Button("Close Menu")) {
            show_menu = false;
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdown() {
    if (!g_Initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    g_Initialized = false;
}

} // namespace ImGuiImpl

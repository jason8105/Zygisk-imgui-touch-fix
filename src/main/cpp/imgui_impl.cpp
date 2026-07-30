#include "imgui_impl.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_android.h"
#include <android/log.h>
#include <GLES3/gl3.h>

#define LOG_TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

static bool g_Initialized = false;
static bool g_ShowMenu = true;

namespace ImGuiImpl {

void init() {
    if (g_Initialized) return;
    LOGD("Initializing ImGui Environment");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui_ImplAndroid_Init(nullptr);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(3.0f); // High-DPI scaling for mobile screens

    g_Initialized = true;
    LOGD("ImGui Environment Initialized successfully");
}

void render() {
    if (!g_Initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    if (g_ShowMenu) {
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Zygisk Universal ImGui Menu", &g_ShowMenu, ImGuiWindowFlags_NoSavedSettings);
        
        ImGui::Text("Universal Touch-Fixed ImGui Menu");
        ImGui::Separator();
        ImGui::Text("Engine: Universal (Unity, Unreal, Native C++)");
        
        static float sliderVal = 50.0f;
        ImGui::SliderFloat("Configuration Slider", &sliderVal, 0.0f, 100.0f);

        static bool toggleVal = true;
        ImGui::Checkbox("Enable Universal Feature", &toggleVal);

        if (ImGui::Button("Reset Settings")) {
            sliderVal = 50.0f;
            toggleVal = true;
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdown() {
    if (!g_Initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    g_Initialized = false;
}

} // namespace ImGuiImpl

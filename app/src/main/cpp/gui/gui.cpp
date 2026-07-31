#include "gui.h"
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include <GLES3/gl3.h>

static bool g_Initialized = false;

namespace GUI {

void Init(int width, int height) {
    if (g_Initialized) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.DisplaySize = ImVec2((float)width, (float)height);
    
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(2.0f);

    ImGui_ImplOpenGL3_Init("#version 300 es");
    g_Initialized = true;
}

void UpdateDisplaySize(int width, int height) {
    if (!g_Initialized) return;
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
}

void Render() {
    if (!g_Initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
    ImGui::Begin("Zygisk Universal ImGui Menu");

    ImGui::Text("Engine Touch & Graphics Hooks Active!");
    ImGui::Separator();

    static bool feature_esp = false;
    static bool feature_aim = false;
    static float fov = 90.0f;

    ImGui::Checkbox("Enable Visual Overlay", &feature_esp);
    ImGui::Checkbox("Enable Touch Assist", &feature_aim);
    ImGui::SliderFloat("FOV Scale", &fov, 30.0f, 120.0f);

    if (ImGui::Button("Reset Configuration")) {
        feature_esp = false;
        feature_aim = false;
        fov = 90.0f;
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace GUI

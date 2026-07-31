#include "menu.h"
#include "imgui.h"

static bool g_MenuOpen = true;
static bool g_FeatureGodMode = false;
static bool g_FeatureUnlimitedAmmo = false;
static float g_FeatureSpeedMultiplier = 1.0f;
static float g_FeatureFov = 90.0f;

namespace Menu {

void Init() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.WindowBorderSize = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.94f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.35f, 0.48f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.30f, 0.42f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.35f, 0.48f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.30f, 0.42f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.15f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.20f, 0.30f, 1.00f);
}

bool IsOpen() {
    return g_MenuOpen;
}

void Render() {
    // Floating Toggle Button
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(100.0f, 40.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Toggle", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    if (ImGui::Button(g_MenuOpen ? "Hide Menu" : "Show Menu", ImVec2(-1, -1))) {
        g_MenuOpen = !g_MenuOpen;
    }
    ImGui::End();

    if (!g_MenuOpen) return;

    ImGui::SetNextWindowPos(ImVec2(150.0f, 100.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(450.0f, 320.0f), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Zygisk Universal ImGui Menu", &g_MenuOpen)) {
        if (ImGui::BeginTabBar("MenuTabBar")) {
            if (ImGui::BeginTabItem("Main")) {
                ImGui::Spacing();
                ImGui::Checkbox("God Mode", &g_FeatureGodMode);
                ImGui::Checkbox("Unlimited Ammo", &g_FeatureUnlimitedAmmo);
                ImGui::Spacing();
                ImGui::SliderFloat("Speed Multiplier", &g_FeatureSpeedMultiplier, 1.0f, 10.0f, "%.1fx");
                ImGui::SliderFloat("FOV Camera", &g_FeatureFov, 60.0f, 120.0f, "%.0f deg");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Touch Status")) {
                ImGui::Spacing();
                ImGuiIO& io = ImGui::GetIO();
                ImGui::Text("Universal Touch Status: ACTIVE");
                ImGui::Separator();
                ImGui::Text("Touch Pos X: %.1f, Y: %.1f", io.MousePos.x, io.MousePos.y);
                ImGui::Text("WantCaptureMouse: %s", io.WantCaptureMouse ? "TRUE (Consumed)" : "FALSE (Passed)");
                ImGui::Text("Display Resolution: %.0f x %.0f", io.DisplaySize.x, io.DisplaySize.y);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("About")) {
                ImGui::Spacing();
                ImGui::Text("Universal Zygisk ImGui Menu");
                ImGui::Text("Supports Unity, Unreal, and Native C++ games.");
                ImGui::Text("Target Compatibility: Magisk v24.0 - v26.x");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

} // namespace Menu

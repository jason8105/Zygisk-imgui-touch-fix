#include "menu.hpp"
#include "../imgui/imgui.h"

namespace Menu {

static bool g_ShowMenu = true;
static bool g_FeatureToggle1 = false;
static float g_FeatureSlider = 1.0f;

void Render() {
    if (!g_ShowMenu) return;

    ImGui::SetNextWindowSize(ImVec2(450, 320), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Universal ImGui Menu (Zygisk)", &g_ShowMenu)) {
        ImGui::Text("Universal Touch & Render Active across Game Engines!");
        ImGui::Separator();

        ImGui::Checkbox("Enable Mod Feature 1", &g_FeatureToggle1);
        ImGui::SliderFloat("Feature Multiplier", &g_FeatureSlider, 0.1f, 10.0f);

        if (ImGui::Button("Close Menu")) {
            g_ShowMenu = false;
        }
    }
    ImGui::End();
}

} // namespace Menu

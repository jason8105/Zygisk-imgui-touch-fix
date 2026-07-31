#include "gui.h"
#include "../imgui/imgui.h"

static bool show_demo_window = false;
static bool menu_open = true;

void RenderGui() {
    if (!menu_open) return;

    ImGui::SetNextWindowSize(ImVec2(360, 260), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Universal Zygisk Menu", &menu_open)) {
        ImGui::Text("Engine: Universal (Unity/Unreal/Native)");
        ImGui::Text("Status: Touch Interception Active");
        ImGui::Separator();

        static bool feature1 = true;
        static bool feature2 = false;
        static float slider_val = 2.5f;

        ImGui::Checkbox("Feature Alpha", &feature1);
        ImGui::Checkbox("Feature Beta", &feature2);
        ImGui::SliderFloat("Value Scale", &slider_val, 0.0f, 10.0f);

        if (ImGui::Button("Toggle ImGui Demo")) {
            show_demo_window = !show_demo_window;
        }
    }
    ImGui::End();

    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
}

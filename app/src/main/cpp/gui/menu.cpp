#include "menu.h"
#include "imgui.h"
#include "imgui_impl_gles3.h"
#include <GLES3/gl3.h>
#include <android/log.h>

#define LOG_TAG "ZygiskMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static bool g_Initialized = false;
static bool g_ShowMenu = true;
static bool g_GodMode = false;
static bool g_UnlimitedAmmo = false;
static float g_Speed = 1.0f;

void init_menu(int width, int height) {
    if (g_Initialized) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(2.5f);
    style.WindowRounding = 8.0f;

    ImGui_ImplGLES3_Init("#version 300 es");
    g_Initialized = true;
    LOGI("ImGui menu initialized (%dx%d)", width, height);
}

void render_menu() {
    if (!g_Initialized) return;

    ImGui_ImplGLES3_NewFrame();
    ImGui::NewFrame();

    if (g_ShowMenu) {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver());
        ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver());

        if (ImGui::Begin("Universal ImGui Mod Menu", &g_ShowMenu)) {
            ImGui::Text("Universal Touch Engine Active!");
            ImGui::Separator();

            ImGui::Checkbox("God Mode", &g_GodMode);
            ImGui::Checkbox("Unlimited Ammo", &g_UnlimitedAmmo);
            ImGui::SliderFloat("Speed", &g_Speed, 1.0f, 10.0f);

            ImGui::Separator();
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplGLES3_RenderDrawData(ImGui::GetDrawData());
}

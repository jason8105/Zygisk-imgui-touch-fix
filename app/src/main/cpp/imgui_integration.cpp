#include "imgui_integration.h"
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>

static bool g_Initialized = false;
static int g_ScreenWidth = 0;
static int g_ScreenHeight = 0;

namespace ImGuiIntegration {

void Init(int width, int height) {
    if (g_Initialized) return;
    
    LOGD("Initializing ImGui for Android (Universal Touch Fix)");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    
    g_ScreenWidth = width > 0 ? width : 1920;
    g_ScreenHeight = height > 0 ? height : 1080;
    io.DisplaySize = ImVec2((float)g_ScreenWidth, (float)g_ScreenHeight);

    ImGui_ImplAndroid_Init(nullptr);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    ImGui::StyleColorsDark();
    g_Initialized = true;
    LOGD("ImGui initialized successfully");
}

void Render() {
    if (!g_Initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    // Universal Demo / Floating Menu Window
    ImGui::Begin("Zygisk Universal Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Universal Touch-Fixed ImGui Menu");
    ImGui::Text("Running on Magisk Zygisk (v24-26)");
    
    static float sliderVal = 0.5f;
    ImGui::SliderFloat("Value", &sliderVal, 0.0f, 1.0f);
    
    static bool toggleVal = true;
    ImGui::Checkbox("Enable Feature", &toggleVal);

    if (ImGui::Button("Reset")) {
        sliderVal = 0.5f;
        toggleVal = true;
    }
    
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool HandleInput(AInputEvent* event) {
    if (!g_Initialized) return false;

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;

        float x = AMotionEvent_getX(event, pointerIndex);
        float y = AMotionEvent_getY(event, pointerIndex);

        ImGuiIO& io = ImGui::GetIO();
        
        switch (maskedAction) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                io.AddMousePosEvent(x, y);
                io.AddMouseButtonEvent(0, true);
                break;
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                io.AddMousePosEvent(x, y);
                io.AddMouseButtonEvent(0, false);
                break;
            case AMOTION_EVENT_ACTION_MOVE:
                io.AddMousePosEvent(x, y);
                break;
            default:
                break;
        }

        // Consume touch event if ImGui wants capture
        if (io.WantCaptureMouse) {
            return true;
        }
    }
    return false;
}

} // namespace ImGuiIntegration

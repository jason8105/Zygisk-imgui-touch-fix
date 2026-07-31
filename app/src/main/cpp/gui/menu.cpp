#include "menu.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <GLES3/gl3.h>
#include <android/input.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "UniversalMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static bool g_Initialized = false;
static bool g_MenuVisible = true;
static bool g_FeatureESP = true;
static bool g_FeatureAimbot = false;
static float g_FOV = 90.0f;
static float g_SpeedMultiplier = 1.0f;

static const char* DetectGameEngine() {
    if (dlopen("libunity.so", RTLD_NOLOAD)) return "Unity Engine";
    if (dlopen("libUE4.so", RTLD_NOLOAD) || dlopen("libUnreal.so", RTLD_NOLOAD)) return "Unreal Engine";
    if (dlopen("libgodot_android.so", RTLD_NOLOAD)) return "Godot Engine";
    if (dlopen("libcocos2d.so", RTLD_NOLOAD)) return "Cocos2d Engine";
    return "Native C++ Engine";
}

void Menu::OnSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)width, (float)height);

            ImGui::StyleColorsDark();
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 8.0f;
            style.FrameRounding = 4.0f;

            ImGui_ImplOpenGL3_Init("#version 300 es");
            g_Initialized = true;
            LOGI("ImGui initialized successfully with screen dimensions (%d x %d)", width, height);
        }
    }

    if (g_Initialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        if (width > 0 && height > 0) {
            ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
        }

        Render();
    }
}

bool Menu::HandleTouch(int action, float x, float y) {
    if (!g_Initialized) return false;

    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    bool isDown = (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN);
    bool isUp = (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP || action == AMOTION_EVENT_ACTION_CANCEL);

    if (isDown) {
        io.AddMouseButtonEvent(0, true);
    } else if (isUp) {
        io.AddMouseButtonEvent(0, false);
    }

    return g_MenuVisible && io.WantCaptureMouse;
}

void Menu::Render() {
    if (!g_Initialized) return;

    // Save OpenGL ES state
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (g_MenuVisible) {
        ImGui::SetNextWindowSize(ImVec2(400, 320), ImGuiCond_FirstUseEver);
        ImGui::Begin("Universal Game Overlay", &g_MenuVisible, ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Detected Engine: %s", DetectGameEngine());
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Touch Mode: %s", ImGui::GetIO().WantCaptureMouse ? "Captured by ImGui" : "Passed to Game");
        ImGui::Separator();

        ImGui::Checkbox("Enable ESP", &g_FeatureESP);
        ImGui::Checkbox("Enable Aimbot", &g_FeatureAimbot);
        ImGui::SliderFloat("FOV Angle", &g_FOV, 30.0f, 180.0f);
        ImGui::SliderFloat("Move Speed", &g_SpeedMultiplier, 1.0f, 5.0f);

        ImGui::Separator();
        if (ImGui::Button("Hide Menu Window")) {
            g_MenuVisible = false;
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Restore OpenGL ES state
    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
}

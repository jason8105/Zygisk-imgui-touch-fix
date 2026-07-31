#include "render_hook.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_android.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../hook/plt_hook.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>

#define LOG_TAG "RenderHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_Initialized = false;
static bool g_ShowMenu = true;

static void SetupImGui(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (width <= 0 || height <= 0) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui_ImplAndroid_InitPlatformInterface(nullptr);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    io.DisplaySize = ImVec2((float)width, (float)height);
    g_Initialized = true;
    LOGI("ImGui initialized successfully (%dx%d)", width, height);
}

static EGLBoolean my_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        SetupImGui(dpy, surface);
    } else {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        if (width > 0 && height > 0) {
            ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        if (g_ShowMenu) {
            ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
            ImGui::Begin("Universal Zygisk Menu", &g_ShowMenu);

            ImGui::Text("Universal Engine Support (Unity / Unreal / Native)");
            ImGui::Separator();

            static bool feature_esp = false;
            static bool feature_aim = false;
            static float fov_value = 90.0f;

            ImGui::Checkbox("ESP Overlay", &feature_esp);
            ImGui::Checkbox("Aimbot Assistance", &feature_aim);
            ImGui::SliderFloat("FOV Field", &fov_value, 30.0f, 180.0f);

            if (ImGui::Button("Hide Menu")) {
                g_ShowMenu = false;
            }

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

namespace RenderHook {
    void Init() {
        PltHook::HookSymbol("libEGL.so", "eglSwapBuffers", (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
        LOGI("EGL SwapBuffers hook installed.");
    }
}

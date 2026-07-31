#include "menu.h"
#include "touch.h"
#include "hook.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLES3/gl3.h>
#include <android/log.h>

#define LOG_TAG "ZygiskImGuiMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*t_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
static t_eglSwapBuffers orig_eglSwapBuffers = nullptr;

static bool g_Initialized = false;

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

            ImGui::StyleColorsDark();
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(2.0f);

            ImGui_ImplOpenGL3_Init("#version 300 es");
            g_Initialized = true;
            LOGI("ImGui initialized successfully with resolution: %dx%d", width, height);
        }
    }

    if (g_Initialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Universal ImGui Menu (Zygisk)")) {
            ImGui::Text("Universal Touch Status: ACTIVE");
            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Text("Mouse/Touch Pos: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);
            ImGui::Text("WantCaptureMouse: %s", io.WantCaptureMouse ? "TRUE" : "FALSE");

            static bool feature_toggle = false;
            ImGui::Checkbox("Enable Demo Feature", &feature_toggle);

            static float feature_slider = 50.0f;
            ImGui::SliderFloat("Value Adjuster", &feature_slider, 0.0f, 100.0f);
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    if (!orig_eglSwapBuffers) {
        void* libegl = dlopen("libEGL.so", RTLD_NOW);
        if (libegl) {
            orig_eglSwapBuffers = (t_eglSwapBuffers)dlsym(libegl, "eglSwapBuffers");
        }
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

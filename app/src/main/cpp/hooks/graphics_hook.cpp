#include "graphics_hook.hpp"
#include "../gui/menu.hpp"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_gles3.h"
#include "hook_utils.hpp"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace GraphicsHook {

static bool g_Initialized = false;
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

EGLBoolean Hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)width, (float)height);
            io.IniFilename = nullptr;

            ImGui::StyleColorsDark();
            ImGui_ImplGLES3_Init("#version 300 es");
            g_Initialized = true;
            LOGI("ImGui initialized: %dx%d", width, height);
        }
    }

    if (g_Initialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui_ImplGLES3_NewFrame();
        ImGui::NewFrame();

        Menu::Render();

        ImGui::Render();
        ImGui_ImplGLES3_RenderDrawData(ImGui::GetDrawData());
    }

    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

bool Init() {
    return HookUtils::HookSymbol(nullptr, "eglSwapBuffers", (void*)Hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
}

} // namespace GraphicsHook

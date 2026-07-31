#include "graphics_hook.h"
#include "hook_engine.h"
#include "touch_hook.h"
#include "menu.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <dlfcn.h>
#include <GLES3/gl3.h>
#include <android/log.h>

#define LOG_TAG "ZygiskGraphicsHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;
static bool g_ImGuiInitialized = false;

namespace GraphicsHook {

EGLBoolean Hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!orig_eglSwapBuffers) {
        void* handle = dlopen("libEGL.so", RTLD_NOW);
        if (handle) {
            orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(handle, "eglSwapBuffers");
        }
    }

    // Refresh touch PLT hooks for dynamically loaded game libraries
    TouchHook::InstallHooks();

    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (width > 0 && height > 0) {
        if (!g_ImGuiInitialized) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)width, (float)height);
            io.IniFilename = nullptr;

            ImGui_ImplOpenGL3_Init("#version 300 es");
            Menu::Init();
            g_ImGuiInitialized = true;
            LOGI("ImGui initialized with screen size: %dx%d", width, height);
        } else {
            ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        Menu::Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

void InstallHooks() {
    void* handle = dlopen("libEGL.so", RTLD_NOW);
    if (handle) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(handle, "eglSwapBuffers");
    }

    HookEngine::PltHookAllModules("eglSwapBuffers", (void*)Hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    LOGI("Graphics EGL hooks installed successfully.");
}

} // namespace GraphicsHook

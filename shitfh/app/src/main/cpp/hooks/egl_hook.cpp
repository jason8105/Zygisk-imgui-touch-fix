#include "egl_hook.h"
#include "plt_hook.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <dlfcn.h>

#define LOG_TAG "UniversalEGLHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_ImGuiInitialized = false;

static void setup_imgui_style() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.13f, 0.85f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.5f, 0.8f, 0.8f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.55f, 0.85f, 1.0f);
}

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

            setup_imgui_style();
            ImGui_ImplOpenGL3_Init("#version 300 es");
            g_ImGuiInitialized = true;
            LOGI("ImGui Context initialized (%dx%d)", width, height);
        }
    }

    if (g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Render Universal Menu Interface
        ImGui::SetNextWindowSize(ImVec2(350, 220), ImGuiCond_FirstUseEver);
        ImGui::Begin("Universal Zygisk Overlay", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Universal Engine ImGui Menu");
        ImGui::Separator();
        ImGui::Text("FPS: %.1f", io.Framerate);

        static bool feature_esp = false;
        static bool feature_aim = false;
        ImGui::Checkbox("Enable ESP Overlay", &feature_esp);
        ImGui::Checkbox("Enable Aim Assist", &feature_aim);

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

void init_universal_egl_hooks(JNIEnv* env) {
    void* handle = dlopen("libEGL.so", RTLD_NOW);
    if (handle) {
        orig_eglSwapBuffers = reinterpret_cast<eglSwapBuffers_t>(dlsym(handle, "eglSwapBuffers"));
        dlclose(handle);
    }

    plt_hook_symbol(nullptr, "eglSwapBuffers", reinterpret_cast<void*>(hook_eglSwapBuffers));
    LOGI("Universal EGL SwapBuffers Hook registered.");
}

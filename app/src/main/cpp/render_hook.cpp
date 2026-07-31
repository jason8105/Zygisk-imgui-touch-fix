#include "render_hook.h"
#include "hook_utils.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "RenderHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_Initialized = false;
static int g_Width = 0;
static int g_Height = 0;

static void SetupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)g_Width, (float)g_Height);
    
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 100");
    g_Initialized = true;
}

static void DrawMenu() {
    ImGui::Begin("Universal ImGui Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Zygisk ImGui Universal Touch Fix");
    ImGui::Text("Screen Size: %dx%d", g_Width, g_Height);
    
    static bool feature1 = false;
    static bool feature2 = true;
    static float sliderVal = 50.0f;

    ImGui::Checkbox("Feature Toggle 1", &feature1);
    ImGui::Checkbox("Feature Toggle 2", &feature2);
    ImGui::SliderFloat("Slider Value", &sliderVal, 0.0f, 100.0f);

    ImGui::End();
}

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &g_Width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_Height);

    if (!g_Initialized) {
        SetupImGui();
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)g_Width, (float)g_Height);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    DrawMenu();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surface);
}

namespace RenderHook {

void Init() {
    void* egl_lib = dlopen("libEGL.so", RTLD_NOW);
    if (egl_lib) {
        void* swap_buffers = dlsym(egl_lib, "eglSwapBuffers");
        if (swap_buffers) {
            DobbyHook(swap_buffers, (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("eglSwapBuffers hooked successfully");
        }
    }
}

} // namespace RenderHook

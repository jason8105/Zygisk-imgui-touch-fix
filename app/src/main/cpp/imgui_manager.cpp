#include "imgui_manager.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "touch_hook.h"
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "ImGuiManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*t_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
static t_eglSwapBuffers orig_eglSwapBuffers = nullptr;

static bool g_Initialized = false;
static int g_Width = 0;
static int g_Height = 0;

extern "C" extern void HookSymbol(void* target, void* replace, void** origin);

static EGLBoolean Hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_Initialized) {
        eglQuerySurface(dpy, surface, EGL_WIDTH, &g_Width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_Height);

        if (g_Width > 0 && g_Height > 0) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)g_Width, (float)g_Height);

            ImGui::StyleColorsDark();
            ImGui_ImplOpenGL3_Init("#version 300 es");

            g_Initialized = true;
            LOGI("ImGui initialized: %dx%d", g_Width, g_Height);
        }
    }

    if (g_Initialized) {
        eglQuerySurface(dpy, surface, EGL_WIDTH, &g_Width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_Height);
        ImGui::GetIO().DisplaySize = ImVec2((float)g_Width, (float)g_Height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);

        ImGui::Begin("Universal Game Engine ImGui Menu");
        ImGui::Text("Engine: Universal (Unity / Unreal / Native)");
        ImGui::Text("Touch Status: Fixed & Capturing");
        ImGui::Separator();

        static bool showDemo = false;
        ImGui::Checkbox("Show Demo Window", &showDemo);

        if (ImGui::Button("Test Touch Capture")) {
            LOGI("ImGui Button Clicked!");
        }

        ImGui::End();

        if (showDemo) {
            ImGui::ShowDemoWindow(&showDemo);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers(dpy, surface);
}

namespace ImGuiManager {

void Init(JNIEnv* env) {
    LOGI("Initializing ImGui eglSwapBuffers Hook...");
    
    void* eglHandle = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    if (eglHandle) {
        void* swapBuffers = dlsym(eglHandle, "eglSwapBuffers");
        if (swapBuffers) {
            HookSymbol(swapBuffers, (void*)Hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("eglSwapBuffers hooked successfully");
        }
    }
}

void OnEglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    Hooked_eglSwapBuffers(dpy, surface);
}

} // namespace ImGuiManager

#include <jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <thread>
#include <chrono>

#include "zygisk.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "touch.h"
#include "hook_utils.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_ImGuiInitialized = false;
static int g_Width = 0;
static int g_Height = 0;

static void RenderMenu() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin("Zygisk Universal Menu (Magisk 24-26)", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Status: Universal Touch Hook Active");
    ImGui::Separator();

    static bool feature1 = false;
    static float speed = 1.0f;
    
    ImGui::Checkbox("Enable Crosshair Overlay", &feature1);
    ImGui::SliderFloat("Game Speed Multiplier", &speed, 0.1f, 5.0f);

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            g_Width = width;
            g_Height = height;

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)width, (float)height);

            ImGui::StyleColorsDark();
            ImGui_ImplOpenGL3_Init("#version 300 es");

            UniversalTouch::InitHooks();
            g_ImGuiInitialized = true;
            LOGI("ImGui Context & OpenGL3 Backend Initialized (%dx%d)", width, height);
        }
    } else {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        if (width > 0 && height > 0) {
            ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
        }
        RenderMenu();
    }

    if (orig_eglSwapBuffers) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    return EGL_TRUE;
}

class UniversalModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            void* sym = find_library_symbol("libEGL.so", "eglSwapBuffers");
            if (sym) {
                orig_eglSwapBuffers = (eglSwapBuffers_t)sym;
                LOGI("EGL SwapBuffers hook resolved.");
            }
        }).detach();
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalModule)

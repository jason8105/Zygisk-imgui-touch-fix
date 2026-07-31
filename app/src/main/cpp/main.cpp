#include <jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/input.h>
#include <android/log.h>
#include <dlfcn.h>
#include <string>

#include "zygisk.hpp"
#include "hook_utils.h"
#include "touch.h"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static bool g_ImGuiInitialized = false;
static int g_Width = 0;
static int g_Height = 0;

typedef EGLBoolean (*t_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
static t_eglSwapBuffers orig_eglSwapBuffers = nullptr;

typedef int (*t_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
static t_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;

typedef int32_t (*t_AMotionEvent_getAction)(const AInputEvent* motion_event);
static t_AMotionEvent_getAction orig_AMotionEvent_getAction = nullptr;

int32_t hooked_AMotionEvent_getAction(const AInputEvent* motion_event) {
    if (motion_event) {
        UniversalTouch::ProcessInputEvent(const_cast<AInputEvent*>(motion_event));
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return AMOTION_EVENT_ACTION_CANCEL;
        }
    }
    if (orig_AMotionEvent_getAction) {
        return orig_AMotionEvent_getAction(motion_event);
    }
    return AMOTION_EVENT_ACTION_CANCEL;
}

int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int res = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;
    if (res >= 0 && outEvent && *outEvent) {
        UniversalTouch::ProcessInputEvent(*outEvent);
    }
    return res;
}

static void InitImGui(EGLDisplay dpy, EGLSurface surface) {
    EGLint w, h;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);
    g_Width = w;
    g_Height = h;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)g_Width, (float)g_Height);

    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 300 es");
    UniversalTouch::Init();

    g_ImGuiInitialized = true;
    LOGI("ImGui successfully initialized (%dx%d)", g_Width, g_Height);
}

EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        InitImGui(dpy, surface);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Universal ImGui Menu", nullptr)) {
        ImGui::Text("Universal Touch & Overlay Engine");
        ImGui::Separator();
        ImGui::Text("Touch Coordinates: (%.1f, %.1f)", UniversalTouch::GetLastX(), UniversalTouch::GetLastY());
        ImGui::Text("Touch Status: %s", UniversalTouch::IsDown() ? "DOWN" : "UP");
        ImGui::Text("Mouse Capture: %s", ImGui::GetIO().WantCaptureMouse ? "Active (Consuming)" : "Inactive");
        
        static bool featureEnabled = false;
        ImGui::Checkbox("Enable Mod Feature", &featureEnabled);
        
        static float value = 1.0f;
        ImGui::SliderFloat("Multipler", &value, 0.0f, 10.0f);
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surface);
}

class ZygiskImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(zygisk::Api *api, JNIEnv *env) override {
        HookUtils::HookSymbol("libEGL.so", "eglSwapBuffers", (void*)hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
        HookUtils::HookSymbol("libandroid.so", "AInputQueue_getEvent", (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        HookUtils::HookSymbol("libandroid.so", "AMotionEvent_getAction", (void*)hooked_AMotionEvent_getAction, (void**)&orig_AMotionEvent_getAction);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(ZygiskImGuiModule)

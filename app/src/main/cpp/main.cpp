#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <thread>
#include <mutex>
#include <dlfcn.h>

#include "zygisk.hpp"
#include "plt_hook.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Function pointer typedefs
typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_ImGuiInitialized = false;
static bool g_ShowMenu = true;

// UNIVERSAL TOUCH HOOK: Captures touch inputs for Unity, Unreal, and Native engines
static int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) {
        orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(RTLD_DEFAULT, "AInputQueue_getEvent");
    }

    int res = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;

    if (res == 0 && outEvent && *outEvent) {
        int32_t type = AInputEvent_getType(*outEvent);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(*outEvent);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

            float x = AMotionEvent_getX(*outEvent, pointerIndex);
            float y = AMotionEvent_getY(*outEvent, pointerIndex);

            if (g_ImGuiInitialized) {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
                    io.AddMouseButtonEvent(0, false);
                }

                // If ImGui UI wants mouse capture, consume event so game engine won't process it
                if (g_ShowMenu && io.WantCaptureMouse) {
                    AInputQueue_finishEvent(queue, *outEvent, 1);
                    *outEvent = nullptr;
                    return -1;
                }
            }
        }
    }
    return res;
}

static void InitImGui(EGLDisplay dpy, EGLSurface surface) {
    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);

    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 300 es");

    g_ImGuiInitialized = true;
    LOGI("ImGui initialized: %dx%d", w, h);
}

static EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!orig_eglSwapBuffers) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(RTLD_DEFAULT, "eglSwapBuffers");
    }

    // Re-apply PLT hook for AInputQueue_getEvent to catch dynamic dlsym/dlopen by engines
    plt_hook::hook_all_modules("AInputQueue_getEvent", (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);

    if (!g_ImGuiInitialized) {
        InitImGui(dpy, surface);
    } else {
        EGLint w = 0, h = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &w);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);
        if (w > 0 && h > 0) {
            ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);
        }
    }

    if (g_ImGuiInitialized) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        if (g_ShowMenu) {
            ImGui::SetNextWindowSize(ImVec2(380, 260), ImGuiCond_FirstUseEver);
            ImGui::Begin("Zygisk Universal ImGui Menu", &g_ShowMenu);
            ImGui::Text("Universal Touch & Overlay Engine Active");
            ImGui::Separator();
            
            static bool option1 = false;
            static bool option2 = false;
            static float speed = 1.0f;

            ImGui::Checkbox("Universal Touch Processing", &option1);
            ImGui::Checkbox("Game Engine Hooking", &option2);
            ImGui::SliderFloat("Value Multiplier", &speed, 0.1f, 5.0f);

            if (ImGui::Button("Close Menu")) {
                g_ShowMenu = false;
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

static void InstallHooks() {
    LOGI("Installing PLT hooks for eglSwapBuffers and AInputQueue_getEvent...");
    plt_hook::hook_all_modules("eglSwapBuffers", (void*)hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    plt_hook::hook_all_modules("AInputQueue_getEvent", (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
}

class UniversalZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *nice_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (nice_name) {
            LOGI("Zygisk specializing app: %s", nice_name);
            is_target = true; // Inject into target processes
            env->ReleaseStringUTFChars(*args->nice_name, nice_name);
        }

        if (!is_target) {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (is_target) {
            std::thread([]() {
                sleep(2); // Wait for application native window setup
                InstallHooks();
            }).detach();
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool is_target = false;
};

REGISTER_ZYGISK_MODULE(UniversalZygiskModule)

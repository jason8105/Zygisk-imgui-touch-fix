#include <jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/input.h>
#include <android/log.h>
#include <android/native_window.h>
#include <unistd.h>
#include <pthread.h>

#include "zygisk.hpp"
#include "plt_hook.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static bool g_ImGuiInitialized = false;
static bool g_ShowMenu = true;

// Touch State
static float g_LastTouchX = 0.0f;
static float g_LastTouchY = 0.0f;

// Original Function Pointers
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef int (*AInputQueue_preDispatchEvent_t)(AInputQueue* queue, AInputEvent* event);

static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

// Universal Touch Handling across Unity, Unreal, and Native C++ Game Engines
static void process_touch_event(AInputEvent* event) {
    if (!g_ImGuiInitialized || !event) return;

    int32_t type = AInputEvent_getType(event);
    if (type != AINPUT_EVENT_TYPE_MOTION) return;

    int32_t action = AMotionEvent_getAction(event);
    int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
    size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    size_t pointerCount = AMotionEvent_getPointerCount(event);
    if (pointerIndex >= pointerCount) pointerIndex = 0;

    float x = AMotionEvent_getX(event, pointerIndex);
    float y = AMotionEvent_getY(event, pointerIndex);

    g_LastTouchX = x;
    g_LastTouchY = y;

    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);

    if (maskedAction == AMOTION_EVENT_ACTION_DOWN || maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN) {
        io.AddMouseButtonEvent(0, true);
    } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
        io.AddMouseButtonEvent(0, false);
    }
}

int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int res = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;
    if (res == 0 && outEvent != nullptr && *outEvent != nullptr) {
        process_touch_event(*outEvent);
    }
    return res;
}

int hook_AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event) {
    if (event != nullptr) {
        process_touch_event(event);

        if (g_ImGuiInitialized) {
            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse) {
                // Intercept touch from game engine when menu is actively touched
                return 1;
            }
        }
    }
    return orig_AInputQueue_preDispatchEvent ? orig_AInputQueue_preDispatchEvent(queue, event) : 0;
}

static void RenderMenu() {
    if (!g_ShowMenu) return;

    ImGui::SetNextWindowSize(ImVec2(450, 320), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Universal Zygisk Menu", &g_ShowMenu)) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Universal Touch Engine Fix Enabled!");
        ImGui::Separator();

        static bool featureA = false;
        static bool featureB = true;
        static float speedMultiplier = 1.0f;

        ImGui::Checkbox("Demo Toggle 1", &featureA);
        ImGui::Checkbox("Demo Toggle 2", &featureB);
        ImGui::SliderFloat("Speed Factor", &speedMultiplier, 0.5f, 5.0f);

        ImGui::Separator();
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Touch Pos: (%.1f, %.1f)", g_LastTouchX, g_LastTouchY);
        ImGui::Text("Mouse Captured: %s", io.WantCaptureMouse ? "YES (Consumed)" : "NO");

        if (ImGui::Button("Hide Menu")) {
            g_ShowMenu = false;
        }
    }
    ImGui::End();
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    static int check_counter = 0;
    if (check_counter++ % 60 == 0) {
        plt_hook::hook_all_modules("AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        plt_hook::hook_all_modules("AInputQueue_preDispatchEvent", (void*)hook_AInputQueue_preDispatchEvent, (void**)&orig_AInputQueue_preDispatchEvent);
    }

    EGLContext ctx = eglGetCurrentContext();
    if (ctx != EGL_NO_CONTEXT) {
        if (!g_ImGuiInitialized) {
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;

            ImGui_ImplOpenGL3_Init("#version 300 es");
            ImGui::StyleColorsDark();

            g_ImGuiInitialized = true;
            LOGI("ImGui Context & OpenGL3 Initialized Successfully");
        }

        if (g_ImGuiInitialized) {
            EGLint width = 0, height = 0;
            eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
            eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

            if (width > 0 && height > 0) {
                ImGuiIO& io = ImGui::GetIO();
                io.DisplaySize = ImVec2((float)width, (float)height);
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();

            RenderMenu();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

class ZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::ApiTable *table, JNIEnv *env) override {
        this->api = zygisk::Api(table);
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char *nice_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (!nice_name) return;

        LOGI("Target App Process Specialize: %s", nice_name);
        env->ReleaseStringUTFChars(*args->nice_name, nice_name);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        plt_hook::hook_all_modules("eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
        plt_hook::hook_all_modules("AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        plt_hook::hook_all_modules("AInputQueue_preDispatchEvent", (void*)hook_AInputQueue_preDispatchEvent, (void**)&orig_AInputQueue_preDispatchEvent);
    }

private:
    zygisk::Api api{nullptr};
    JNIEnv *env{nullptr};
};

REGISTER_ZYGISK_MODULE(ZygiskModule)

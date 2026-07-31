#include <jni.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/input.h>

#include "zygisk.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "plt_hook.hpp"

#define LOG_TAG "ZygiskTouchMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_ImGuiInitialized = false;
static EGLContext g_EGLContext = EGL_NO_CONTEXT;

int hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) {
        orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(RTLD_NEXT, "AInputQueue_getEvent");
        if (!orig_AInputQueue_getEvent) {
            void* handle = dlopen("libandroid.so", RTLD_NOW);
            if (handle) orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(handle, "AInputQueue_getEvent");
        }
    }

    int res = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;

    if (res == 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

            float x = AMotionEvent_getX(event, pointerIndex);
            float y = AMotionEvent_getY(event, pointerIndex);

            ImGuiIO& io = ImGui::GetIO();
            io.AddMousePosEvent(x, y);

            if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
                io.AddMouseButtonEvent(0, false);
            }

            if (io.WantCaptureMouse) {
                AInputQueue_finishEvent(queue, event, 1);
                *outEvent = nullptr;
                return -1;
            }
        }
    }
    return res;
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!orig_eglSwapBuffers) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(RTLD_NEXT, "eglSwapBuffers");
        if (!orig_eglSwapBuffers) {
            void* handle = dlopen("libEGL.so", RTLD_NOW);
            if (handle) orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(handle, "eglSwapBuffers");
        }
    }

    EGLContext currentContext = eglGetCurrentContext();
    if (currentContext != EGL_NO_CONTEXT) {
        if (!g_ImGuiInitialized || g_EGLContext != currentContext) {
            g_EGLContext = currentContext;
            
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;

            ImGui::StyleColorsDark();
            ImGui_ImplOpenGL3_Init("#version 100");
            g_ImGuiInitialized = true;
            LOGI("ImGui context successfully initialized");
        }

        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        ImGuiIO& io = ImGui::GetIO();
        if (width > 0 && height > 0) {
            io.DisplaySize = ImVec2((float)width, (float)height);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Render Universal Menu Window
        static bool menu_open = true;
        ImGui::SetNextWindowSize(ImVec2(380, 280), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Universal ImGui Menu (Zygisk)", &menu_open)) {
            ImGui::Text("Status: Active");
            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Text("Resolution: %.0fx%.0f", io.DisplaySize.x, io.DisplaySize.y);
            ImGui::Text("Capture Touch: %s", io.WantCaptureMouse ? "Yes" : "No");
            ImGui::Separator();

            static bool feature1 = false;
            ImGui::Checkbox("Universal Feature Mod", &feature1);

            static float speed = 1.0f;
            ImGui::SliderFloat("Value Adjuster", &speed, 0.1f, 5.0f);

            if (ImGui::Button("Reset All Values")) {
                feature1 = false;
                speed = 1.0f;
            }
        }
        ImGui::End();

        ImGui::Render();

        GLint last_program, last_viewport[4];
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        glGetIntegerv(GL_VIEWPORT, last_viewport);

        glViewport(0, 0, width, height);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glUseProgram(last_program);
        glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

static void* hook_thread(void*) {
    for (int i = 0; i < 10; i++) {
        sleep(1 + i);
        PltHook::hook_all_modules("AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        PltHook::hook_all_modules("eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    }
    return nullptr;
}

class UniversalMenuModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        const char* process_name = env->GetStringUTFChars(*args->nice_name, nullptr);
        if (process_name) {
            LOGI("Zygisk injected into process: %s", process_name);
            env->ReleaseStringUTFChars(*args->nice_name, process_name);
            
            pthread_t t;
            pthread_create(&t, nullptr, hook_thread, nullptr);
            pthread_detach(t);
        }
    }

private:
    Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalMenuModule)

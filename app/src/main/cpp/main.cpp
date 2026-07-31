#include <thread>
#include <chrono>
#include <vector>
#include <android/input.h>
#include <android/keycodes.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "zygisk.hpp"
#include "plt_hook.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"

// Original function prototypes
typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static bool g_ImGuiInitialized = false;
static int g_ScreenWidth = 0;
static int g_ScreenHeight = 0;

// Universal Input Hook across all game engines (Unity, Unreal, Native C++)
int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    if (!orig_AInputQueue_getEvent) return -1;

    while (true) {
        int res = orig_AInputQueue_getEvent(queue, outEvent);
        if (res < 0 || !outEvent || !*outEvent) {
            return res;
        }

        AInputEvent* event = *outEvent;
        int32_t type = AInputEvent_getType(event);

        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            size_t pointerCount = AMotionEvent_getPointerCount(event);

            if (pointerCount > 0) {
                float x = AMotionEvent_getX(event, 0);
                float y = AMotionEvent_getY(event, 0);

                if (g_ImGuiInitialized && ImGui::GetCurrentContext() != nullptr) {
                    ImGuiIO& io = ImGui::GetIO();
                    io.AddMousePosEvent(x, y);

                    if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                        io.AddMouseButtonEvent(0, true);
                    } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
                        io.AddMouseButtonEvent(0, false);
                    }

                    // UNIVERSAL TOUCH CONSUMPTION: Swallow input if ImGui wants mouse capture
                    if (io.WantCaptureMouse) {
                        if (orig_AInputQueue_finishEvent) {
                            orig_AInputQueue_finishEvent(queue, event, 1);
                        }
                        continue; // Fetch next event for game engine
                    }
                }
            }
        }

        return res;
    }
}

EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            g_ScreenWidth = width;
            g_ScreenHeight = height;

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr; // Disable ini persistence on mobile

            // High DPI Scaling for Android devices
            float scale = (float)width / 1280.0f;
            if (scale < 1.0f) scale = 1.0f;
            ImGui::GetStyle().ScaleAllSizes(scale);
            io.FontGlobalScale = scale;

            ImGui_ImplOpenGL3_Init("#version 300 es");
            g_ImGuiInitialized = true;
            LOGI("ImGui successfully initialized (%dx%d)", width, height);
        }
    } else {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        if (width > 0 && height > 0) {
            g_ScreenWidth = width;
            g_ScreenHeight = height;
        }
    }

    if (g_ImGuiInitialized && ImGui::GetCurrentContext() != nullptr) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)g_ScreenWidth, (float)g_ScreenHeight);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Render Universal Menu Overlay
        ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(450.0f, 320.0f), ImGuiCond_FirstUseEver);

        ImGui::Begin("Universal Zygisk ImGui Menu");
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Separator();
        ImGui::Text("Universal Touch: FIXED (Unity/Unreal/Native)");
        
        static bool enable_esp = true;
        ImGui::Checkbox("Enable Overlay Feature", &enable_esp);
        
        static float speed_multiplier = 1.0f;
        ImGui::SliderFloat("Speed Multiplier", &speed_multiplier, 0.5f, 5.0f);
        
        if (ImGui::Button("Trigger Action")) {
            LOGI("Button clicked inside ImGui overlay!");
        }
        ImGui::End();

        ImGui::Render();

        // Preserve OpenGL state across game engine renders
        GLint last_program, last_viewport[4], last_array_buffer;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        glGetIntegerv(GL_VIEWPORT, last_viewport);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);

        glViewport(0, 0, g_ScreenWidth, g_ScreenHeight);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Restore OpenGL state
        glUseProgram(last_program);
        glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    }

    return orig_eglSwapBuffers(dpy, surface);
}

static void init_thread() {
    LOGI("Injection thread launched");

    void* libandroid = dlopen("libandroid.so", RTLD_NOW);
    if (libandroid) {
        orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(libandroid, "AInputQueue_getEvent");
        orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)dlsym(libandroid, "AInputQueue_finishEvent");
    }

    void* libegl = dlopen("libEGL.so", RTLD_NOW);
    if (libegl) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(libegl, "eglSwapBuffers");
    }

    std::vector<PLTHook::HookTarget> targets = {
        {"AInputQueue_getEvent", (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent},
        {"eglSwapBuffers", (void*)hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers}
    };

    // Retry loop to ensure hooks are hooked into dynamically loaded game engine libraries
    for (int i = 0; i < 30; ++i) {
        PLTHook::hook_all(targets);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

class ZygiskImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;

        jboolean is_copy;
        const char *process_name = env->GetStringUTFChars(args->nice_name, &is_copy);
        if (process_name) {
            is_target = should_inject(process_name);
            env->ReleaseStringUTFChars(args->nice_name, process_name);
        }

        if (is_target) {
            api->setOption(zygisk::DLCLOSE_MODULE_LOADED);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (is_target) {
            std::thread(init_thread).detach();
        }
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    bool is_target = false;

    bool should_inject(const char* process_name) {
        if (!process_name) return false;
        // Exclude system apps and root managers
        if (strstr(process_name, "com.android.") ||
            strstr(process_name, "system_server") ||
            strstr(process_name, "com.google.android.") ||
            strstr(process_name, "com.topjohnwu.magisk")) {
            return false;
        }
        return true;
    }
};

REGISTER_ZYGISK_MODULE(ZygiskImGuiModule)

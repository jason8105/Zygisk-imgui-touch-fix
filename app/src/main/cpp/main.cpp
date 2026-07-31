#include <jni.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>

#include "zygisk.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "dobby/dobby.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef EGLBoolean (*t_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
typedef int32_t (*t_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*t_AInputQueue_finishEvent)(AInputQueue* queue, AInputEvent* event, int handled);

static t_eglSwapBuffers orig_eglSwapBuffers = nullptr;
static t_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;
static t_AInputQueue_finishEvent orig_AInputQueue_finishEvent = nullptr;

static bool g_ImGuiInitialized = false;
static float g_LastTouchX = 0.0f;
static float g_LastTouchY = 0.0f;
static bool g_LastTouchDown = false;
static bool g_ShowMenu = true;

static bool g_FeatureESP = true;
static bool g_FeatureCrosshair = true;
static float g_CrosshairSize = 12.0f;

static void RenderMenu() {
    if (!g_ShowMenu) return;

    ImGui::SetNextWindowSize(ImVec2(480, 340), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Universal ImGui Menu (Zygisk)", &g_ShowMenu)) {
        ImGui::Text("Universal Engine Hook (Unity, Unreal, Native C++)");
        ImGui::Separator();

        if (ImGui::BeginTabBar("MainTabBar")) {
            if (ImGui::BeginTabItem("Visuals")) {
                ImGui::Checkbox("Draw Crosshair Overlay", &g_FeatureCrosshair);
                if (g_FeatureCrosshair) {
                    ImGui::SliderFloat("Crosshair Size", &g_CrosshairSize, 5.0f, 40.0f);
                }
                ImGui::Checkbox("ESP Box Overlay (Demo)", &g_FeatureESP);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Touch Info")) {
                ImGui::Text("Touch Event Coordinates: (%.1f, %.1f)", g_LastTouchX, g_LastTouchY);
                ImGui::Text("Touch Action Down: %s", g_LastTouchDown ? "YES" : "NO");
                ImGuiIO& io = ImGui::GetIO();
                ImGui::Text("ImGui WantCaptureMouse: %s", io.WantCaptureMouse ? "TRUE" : "FALSE");
                ImGui::Text("Display Resolution: %.0fx%.0f", io.DisplaySize.x, io.DisplaySize.y);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Theme")) {
                if (ImGui::Button("Dark Theme")) ImGui::StyleColorsDark();
                ImGui::SameLine();
                if (ImGui::Button("Classic Theme")) ImGui::StyleColorsClassic();
                ImGui::SameLine();
                if (ImGui::Button("Light Theme")) ImGui::StyleColorsLight();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        if (g_FeatureCrosshair) {
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            ImVec2 center = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
            ImU32 col = IM_COL32(0, 255, 0, 255);
            drawList->AddLine(ImVec2(center.x - g_CrosshairSize, center.y), ImVec2(center.x + g_CrosshairSize, center.y), col, 2.0f);
            drawList->AddLine(ImVec2(center.x, center.y - g_CrosshairSize), ImVec2(center.x, center.y + g_CrosshairSize), col, 2.0f);
        }
    }
    ImGui::End();
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
            io.DisplaySize = ImVec2((float)width, (float)height);

            ImGui::GetStyle().ScaleAllSizes(2.0f);
            ImGui::GetStyle().WindowRounding = 8.0f;
            ImGui::StyleColorsDark();

            ImGui_ImplOpenGL3_Init("#version 300 es");
            g_ImGuiInitialized = true;
            LOGI("ImGui context created for display %dx%d", width, height);
        }
    }

    if (g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        if (width > 0 && height > 0) {
            ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        RenderMenu();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers(dpy, surface);
}

static int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res >= 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);

            g_LastTouchX = x;
            g_LastTouchY = y;

            if (g_ImGuiInitialized) {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    g_LastTouchDown = true;
                    io.AddMouseButtonEvent(0, true);
                } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
                    g_LastTouchDown = false;
                    io.AddMouseButtonEvent(0, false);
                }

                // Universal Touch Capture: Consume touch event when interacting with ImGui
                if (io.WantCaptureMouse) {
                    if (orig_AInputQueue_finishEvent) {
                        orig_AInputQueue_finishEvent(queue, event, 1);
                    }
                    return orig_AInputQueue_getEvent(queue, outEvent);
                }
            }
        }
    }
    return res;
}

static void* hook_worker_thread(void*) {
    LOGI("Universal hook worker thread started");
    bool hookedEGL = false;
    bool hookedInput = false;

    for (int attempt = 0; attempt < 100; ++attempt) {
        if (!hookedEGL) {
            void* handle = dlopen("libEGL.so", RTLD_NOLOAD);
            if (!handle) handle = dlopen("libEGL.so", RTLD_NOW);
            if (handle) {
                void* symbol = dlsym(handle, "eglSwapBuffers");
                if (symbol && DobbyHook(symbol, (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers) == 0) {
                    hookedEGL = true;
                    LOGI("Hooked eglSwapBuffers successfully");
                }
            }
        }

        if (!hookedInput) {
            void* handle = dlopen("libandroid.so", RTLD_NOLOAD);
            if (!handle) handle = dlopen("libandroid.so", RTLD_NOW);
            if (handle) {
                orig_AInputQueue_finishEvent = (t_AInputQueue_finishEvent)dlsym(handle, "AInputQueue_finishEvent");
                void* symbol = dlsym(handle, "AInputQueue_getEvent");
                if (symbol && DobbyHook(symbol, (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent) == 0) {
                    hookedInput = true;
                    LOGI("Hooked AInputQueue_getEvent successfully");
                }
            }
        }

        if (hookedEGL && hookedInput) {
            LOGI("All universal hooks installed!");
            break;
        }

        usleep(500000); // Sleep 500ms between attempts
    }
    return nullptr;
}

class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) return;
        const char *process_name = env->GetStringUTFChars(args->nice_name, nullptr);
        if (process_name) {
            LOGI("preAppSpecialize: %s", process_name);
            api->setOption(zygisk::Option::DLCLOSE_MODULE_FOR_UNLOADED_PROCESS);
            env->ReleaseStringUTFChars(args->nice_name, process_name);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        pthread_t thread;
        pthread_create(&thread, nullptr, hook_worker_thread, nullptr);
        pthread_detach(thread);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(UniversalImGuiModule)

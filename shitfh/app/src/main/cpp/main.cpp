#include <jni.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>

#include "zygisk.hpp"
#include "dobby.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global state
bool g_ImGuiInitialized = false;

// Originals
float (*orig_AMotionEvent_getX)(const AInputEvent* motion_event, size_t pointer_index) = nullptr;
float (*orig_AMotionEvent_getY)(const AInputEvent* motion_event, size_t pointer_index) = nullptr;
int32_t (*orig_AMotionEvent_getAction)(const AInputEvent* motion_event) = nullptr;
int32_t (*orig_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent) = nullptr;

typedef int32_t (*InputConsumer_consume_t)(void* self, void* factory, bool consumeBatched, int64_t frameTime, uint32_t* outSeq, void** outEvent);
InputConsumer_consume_t orig_InputConsumer_consume = nullptr;

EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface) = nullptr;

// Universal Input Handler
void handle_input_event(AInputEvent* event) {
    if (!g_ImGuiInitialized || ImGui::GetCurrentContext() == nullptr) return;

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = orig_AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        float x = orig_AMotionEvent_getX(event, 0);
        float y = orig_AMotionEvent_getY(event, 0);

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMouseButtonEvent(0, true);
        } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
            io.AddMouseButtonEvent(0, false);
        }
    }
}

// Hook functions for inputs
float my_AMotionEvent_getX(const AInputEvent* motion_event, size_t pointer_index) {
    if (g_ImGuiInitialized && ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse) {
        return -11000.0f; // Consume touch by shifting position out-of-bounds safe from clicks
    }
    return orig_AMotionEvent_getX(motion_event, pointer_index);
}

float my_AMotionEvent_getY(const AInputEvent* motion_event, size_t pointer_index) {
    if (g_ImGuiInitialized && ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse) {
        return -11000.0f;
    }
    return orig_AMotionEvent_getY(motion_event, pointer_index);
}

int32_t my_AMotionEvent_getAction(const AInputEvent* motion_event) {
    if (g_ImGuiInitialized && ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse) {
        // We can return cancel to stop active gestures gracefully
        int32_t action = orig_AMotionEvent_getAction(motion_event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        if (actionMasked == AMOTION_EVENT_ACTION_MOVE) {
            return AMOTION_EVENT_ACTION_CANCEL;
        }
    }
    return orig_AMotionEvent_getAction(motion_event);
}

int32_t my_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t result = orig_AInputQueue_getEvent(queue, outEvent);
    if (result >= 0 && outEvent && *outEvent) {
        handle_input_event(*outEvent);
    }
    return result;
}

int32_t my_InputConsumer_consume(void* self, void* factory, bool consumeBatched, int64_t frameTime, uint32_t* outSeq, void** outEvent) {
    int32_t result = orig_InputConsumer_consume(self, factory, consumeBatched, frameTime, outSeq, outEvent);
    if (result == 0 && outEvent && *outEvent) {
        handle_input_event((AInputEvent*)*outEvent);
    }
    return result;
}

// Hook for swap buffers / UI Loop
EGLBoolean my_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    // Back up current state
    GLint last_program, last_active_texture, last_texture;
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);

    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    if (!g_ImGuiInitialized) {
        EGLint width, height;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui::StyleColorsDark();

        // Custom style touches
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 8.0f;
        style.FrameRounding = 4.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.13f, 0.90f);

        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_ImGuiInitialized = true;
    }

    EGLint width, height;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);

    ImGui_ImplOpenGL3_NewFrame();
    
    static auto last_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta = current_time - last_time;
    io.DeltaTime = delta.count() > 0.0f ? delta.count() : 1.0f / 60.0f;
    last_time = current_time;

    ImGui::NewFrame();

    // The Menu Window
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Universal Game Overlay Menu (Zygisk)", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Universal Touch Fix Enabled!");
        ImGui::Separator();
        
        static bool menu_active = true;
        ImGui::Checkbox("Active Controls", &menu_active);

        static float speed = 1.0f;
        ImGui::SliderFloat("Speed Multiplier", &speed, 1.0f, 10.0f);

        static float fov = 90.0f;
        ImGui::SliderFloat("Custom FOV", &fov, 60.0f, 120.0f);

        if (ImGui::Button("Reset to Defaults")) {
            speed = 1.0f;
            fov = 90.0f;
        }
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Restore state
    glUseProgram(last_program);
    glActiveTexture(last_active_texture);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);

    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);

    return orig_eglSwapBuffers(dpy, surface);
}

void start_hook_thread() {
    LOGI("Hooking thread started. Waiting for libraries to load...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Universal eglSwapBuffers hook
    void* libegl = dlopen("libEGL.so", RTLD_NOW);
    if (libegl) {
        void* swap = dlsym(libegl, "eglSwapBuffers");
        if (swap) {
            DobbyHook(swap, (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("Successfully hooked eglSwapBuffers in libEGL.so");
        }
    }

    // Universal inputs hooks
    void* libandroid = dlopen("libandroid.so", RTLD_NOW);
    if (libandroid) {
        void* getX = dlsym(libandroid, "AMotionEvent_getX");
        void* getY = dlsym(libandroid, "AMotionEvent_getY");
        void* getAction = dlsym(libandroid, "AMotionEvent_getAction");
        void* getEvent = dlsym(libandroid, "AInputQueue_getEvent");

        if (getX && getY && getAction) {
            DobbyHook(getX, (void*)my_AMotionEvent_getX, (void**)&orig_AMotionEvent_getX);
            DobbyHook(getY, (void*)my_AMotionEvent_getY, (void**)&orig_AMotionEvent_getY);
            DobbyHook(getAction, (void*)my_AMotionEvent_getAction, (void**)&orig_AMotionEvent_getAction);
            LOGI("Successfully hooked AMotionEvent getters in libandroid.so");
        }
        if (getEvent) {
            DobbyHook(getEvent, (void*)my_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            LOGI("Successfully hooked AInputQueue_getEvent in libandroid.so");
        }
    }

    void* libinput = dlopen("libinput.so", RTLD_NOW);
    if (libinput) {
        // Try resolving the mangled name for InputConsumer::consume
        void* consume = dlsym(libinput, "_ZN7android13InputConsumer7consumeEPNS_27InputEventFactoryInterfaceEbNS_8nsecs_tEPjPPNS_10InputEventE");
        if (consume) {
            DobbyHook(consume, (void*)my_InputConsumer_consume, (void**)&orig_InputConsumer_consume);
            LOGI("Successfully hooked InputConsumer::consume in libinput.so");
        }
    }
}

class MyZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, jclass clazz) override {
        this->api = api;
        this->clazz = clazz;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = api->getArgString(args->process);
        if (process) {
            std::string target_proc = get_target_process();
            if (target_proc == "*" || strcmp(process, target_proc.c_str()) == 0) {
                enable_hook = true;
                LOGI("Target detected: %s. Enabling hooks.", process);
            }
            api->releaseArgString(args->process, process);
        }
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        if (enable_hook) {
            std::thread(start_hook_thread).detach();
        }
    }

private:
    Api *api = nullptr;
    jclass clazz = nullptr;
    bool enable_hook = false;

    std::string get_target_process() {
        std::ifstream file("/data/local/tmp/imgui_target");
        if (file.is_open()) {
            std::string line;
            if (std::getline(file, line)) {
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                if (!line.empty()) {
                    return line;
                }
            }
        }
        return "*"; // Universal
    }
};

REGISTER_ZYGISK_MODULE(MyZygiskModule)

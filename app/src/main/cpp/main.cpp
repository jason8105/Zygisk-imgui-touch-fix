#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>

#include "zygisk.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define TAG "ZygiskImgui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Universal input function pointer types for Hooking AInputQueue or native dispatches
typedef int (*AInputQueue_preDispatchEvent_t)(void* queue, void* event);
typedef int (*AInputQueue_finishEvent_t)(void* queue, void* event, int handled);

static AInputQueue_preDispatchEvent_t orig_AInputQueue_preDispatchEvent = nullptr;

// AInputEvent methods (resolved dynamically)
typedef int32_t (*AInputEvent_getType_t)(const void* event);
typedef int32_t (*AInputEvent_getSource_t)(const void* event);
typedef size_t (*AMotionEvent_getPointerCount_t)(const void* event);
typedef int32_t (*AMotionEvent_getAction_t)(const void* event);
typedef float (*AMotionEvent_getX_t)(const void* event, size_t pointer_index);
typedef float (*AMotionEvent_getY_t)(const void* event, size_t pointer_index);

static AInputEvent_getType_t p_AInputEvent_getType = nullptr;
static AInputEvent_getSource_t p_AInputEvent_getSource = nullptr;
static AMotionEvent_getPointerCount_t p_AMotionEvent_getPointerCount = nullptr;
static AMotionEvent_getAction_t p_AMotionEvent_getAction = nullptr;
static AMotionEvent_getX_t p_AMotionEvent_getX = nullptr;
static AMotionEvent_getY_t p_AMotionEvent_getY = nullptr;

static bool imgui_initialized = false;
static bool show_menu = true;

// EGL swap buffers hook type
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

void init_imgui() {
    if (imgui_initialized) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 300 es");
    imgui_initialized = true;
    LOGD("ImGui initialized successfully via Zygisk hook.");
}

int hook_AInputQueue_preDispatchEvent(void* queue, void* event) {
    if (event && p_AInputEvent_getType && p_AInputEvent_getSource && p_AMotionEvent_getAction) {
        int32_t eventType = p_AInputEvent_getType(event);
        // AINPUT_EVENT_TYPE_MOTION = 2
        if (eventType == 2) {
            int32_t action = p_AMotionEvent_getAction(event);
            int actionMasked = action & 0xff; // AMOTION_EVENT_ACTION_MASK
            
            size_t pointerCount = p_AMotionEvent_getPointerCount ? p_AMotionEvent_getPointerCount(event) : 1;
            if (pointerCount > 0 && p_AMotionEvent_getX && p_AMotionEvent_getY) {
                float x = p_AMotionEvent_getX(event, 0);
                float y = p_AMotionEvent_getY(event, 0);

                ImGuiIO& io = ImGui::GetIO();
                if (actionMasked == 0 /* DOWN */ || actionMasked == 2 /* MOVE */ || actionMasked == 5 /* POINTER_DOWN */) {
                    io.AddMousePosEvent(x, y);
                    io.AddMouseButtonEvent(0, true);
                } else if (actionMasked == 1 /* UP */ || actionMasked == 3 /* CANCEL */ || actionMasked == 6 /* POINTER_UP */) {
                    io.AddMousePosEvent(x, y);
                    io.AddMouseButtonEvent(0, false);
                }

                if (show_menu && io.WantCaptureMouse) {
                    // Consume touch event so underlying game engine doesn't process it
                    return 1; 
                }
            }
        }
    }
    if (orig_AInputQueue_preDispatchEvent) {
        return orig_AInputQueue_preDispatchEvent(queue, event);
    }
    return 0;
}

EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!imgui_initialized) {
        init_imgui();
    }

    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (width > 0 && height > 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        if (show_menu) {
            ImGui::Begin("Zygisk Universal ImGui Menu", &show_menu);
            ImGui::Text("Universal Touch & Game Engine Fix Active!");
            ImGui::Text("Application resolution: %dx%d", width, height);
            if (ImGui::Button("Toggle Menu State")) {
                // Example action
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers(dpy, surface);
}

class ZygiskImguiModule : public zygisk::Module {
public:
    void on_load(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void pre_app_specialize(void *args) override {
        // Not used heavily here
    }

    void post_app_specialize(void *args) override {
        LOGD("post_app_specialize executed in target app.");

        // Resolve Android input symbols for universal touch input
        void* libandroid = dlopen("libandroid.so", RTLD_LAZY);
        if (libandroid) {
            orig_AInputQueue_preDispatchEvent = (AInputQueue_preDispatchEvent_t)dlsym(libandroid, "AInputQueue_preDispatchEvent");
            p_AInputEvent_getType = (AInputEvent_getType_t)dlsym(libandroid, "AInputEvent_getType");
            p_AInputEvent_getSource = (AInputEvent_getSource_t)dlsym(libandroid, "AInputEvent_getSource");
            p_AMotionEvent_getPointerCount = (AMotionEvent_getPointerCount_t)dlsym(libandroid, "AMotionEvent_getPointerCount");
            p_AMotionEvent_getAction = (AMotionEvent_getAction_t)dlsym(libandroid, "AMotionEvent_getAction");
            p_AMotionEvent_getX = (AMotionEvent_getX_t)dlsym(libandroid, "AMotionEvent_getX");
            p_AMotionEvent_getY = (AMotionEvent_getY_t)dlsym(libandroid, "AMotionEvent_getY");

            if (orig_AInputQueue_preDispatchEvent) {
                api->plt_hook_register("libandroid.so", "AInputQueue_preDispatchEvent", (void*)hook_AInputQueue_preDispatchEvent, (void**)&orig_AInputQueue_preDispatchEvent);
                LOGD("Successfully hooked AInputQueue_preDispatchEvent for universal touch input.");
            }
        }

        // Hook eglSwapBuffers for rendering ImGui
        api->plt_hook_register("libEGL.so", "eglSwapBuffers", (void*)hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
        api->plt_hook_commit();
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(ZygiskImguiModule)

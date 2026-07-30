#include <jni.h>
#include <android/log.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cstdlib>

#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include "dobby.h"
#include "zygisk.hpp"

#define LOG_TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static bool g_Initialized = false;
static int g_ScreenWidth = 0;
static int g_ScreenHeight = 0;

// Original Unity_nativeInjectEvent function pointer
typedef void (*Unity_nativeInjectEvent_t)(JNIEnv* env, jobject thiz, jobject motionEvent);
static Unity_nativeInjectEvent_t o_Unity_nativeInjectEvent = nullptr;

// Dobby hook implementation for Unity's native touch injection point
void hk_Unity_nativeInjectEvent(JNIEnv* env, jobject thiz, jobject motionEvent) {
    if (motionEvent != nullptr) {
        // Find MotionEvent methods
        jclass motionEventClass = env->GetObjectClass(motionEvent);
        if (motionEventClass != nullptr) {
            jmethodID getActionMasked = env->GetMethodID(motionEventClass, "getActionMasked", "()I");
            jmethodID getX = env->GetMethodID(motionEventClass, "getX", "()F");
            jmethodID getY = env->GetMethodID(motionEventClass, "getY", "()F");

            if (getActionMasked && getX && getY) {
                int action = env->CallIntMethod(motionEvent, getActionMasked);
                float x = env->CallFloatMethod(motionEvent, getX);
                float y = env->CallFloatMethod(motionEvent, getY);

                ImGuiIO& io = ImGui::GetIO();
                
                int imgui_action = -1;
                if (action == 0 /* ACTION_DOWN */ || action == 5 /* ACTION_POINTER_DOWN */) {
                    imgui_action = 0;
                } else if (action == 1 /* ACTION_UP */ || action == 6 /* ACTION_POINTER_UP */) {
                    imgui_action = 1;
                } else if (action == 2 /* ACTION_MOVE */) {
                    imgui_action = 2;
                }

                if (imgui_action != -1) {
                    io.AddMousePosEvent(x, y);
                    io.AddMouseButtonEvent(0, imgui_action == 0 || imgui_action == 2);
                }

                // If ImGui wants to capture touch, consume/block it from reaching Unity
                if (g_Initialized && io.WantCaptureMouse) {
                    env->DeleteLocalRef(motionEventClass);
                    return; 
                }
            }
            env->DeleteLocalRef(motionEventClass);
        }
    }

    if (o_Unity_nativeInjectEvent) {
        o_Unity_nativeInjectEvent(env, thiz, motionEvent);
    }
}

// Hooking EGL swap buffers to render ImGui
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);
static eglSwapBuffers_t o_eglSwapBuffers = nullptr;

EGLBoolean hk_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (width > 0 && height > 0) {
        g_ScreenWidth = width;
        g_ScreenHeight = height;
    }

    if (!g_Initialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)g_ScreenWidth, (float)g_ScreenHeight);
        
        ImGui_ImplAndroid_Init(nullptr);
        ImGui_ImplOpenGL3_Init("#version 300 es");

        ImGui::StyleColorsDark();
        g_Initialized = true;
        LOGD("ImGui initialized successfully via EGL hook.");
    }

    if (g_Initialized) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame(g_ScreenWidth, g_ScreenHeight);
        ImGui::NewFrame();

        // Render ImGui Menu
        ImGui::Begin("Zygisk Unity ImGui Menu");
        ImGui::Text("Touch-fixed ImGui running inside Unity!");
        ImGui::Text("Resolution: %dx%d", g_ScreenWidth, g_ScreenHeight);
        if (ImGui::Button("Test Button")) {
            LOGD("ImGui Button Clicked!");
        }
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, g_ScreenWidth, g_ScreenHeight);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetData());
    }

    return o_eglSwapBuffers(dpy, surface);
}

// Thread to hook libunity.so dynamically once loaded
void* hook_thread(void*) {
    LOGD("Waiting for libunity.so to load...");
    void* handle = nullptr;
    while (!handle) {
        handle = dlopen("libunity.so", RTLD_NOLOAD | RTLD_LAZY);
        if (!handle) {
            usleep(100000);
        }
    }
    LOGD("libunity.so loaded at %p", handle);

    // Find Unity_nativeInjectEvent
    void* sym = dlsym(handle, "Unity_nativeInjectEvent");
    if (sym) {
        LOGD("Found Unity_nativeInjectEvent at %p", sym);
        DobbyHook(sym, (void*)hk_Unity_nativeInjectEvent, (void**)&o_Unity_nativeInjectEvent);
    } else {
        LOGE("Failed to find Unity_nativeInjectEvent symbol!");
    }

    // Also hook eglSwapBuffers for rendering
    void* eglHandle = dlopen("libEGL.so", RTLD_NOLOAD | RTLD_LAZY);
    if (eglHandle) {
        void* eglSwapBuffersSym = dlsym(eglHandle, "eglSwapBuffers");
        if (eglSwapBuffersSym) {
            DobbyHook(eglSwapBuffersSym, (void*)hk_eglSwapBuffers, (void**)&o_eglSwapBuffers);
            LOGD("Hooked eglSwapBuffers successfully.");
        }
    }

    return nullptr;
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        // Not used here, we use post
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        pthread_t pt;
        pthread_create(&pt, nullptr, hook_thread, nullptr);
    }

private:
    zygisk::Api* api = nullptr;
    JNIEnv* env = nullptr;
};

REGISTER_ZYGISK_MODULE(MyModule)

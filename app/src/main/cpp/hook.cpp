#include "hook.h"
#include <dlfcn.h>
#include <android/log.h>
#include <android/input.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "ZygiskImGuiUniversal"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static bool g_ImGuiInitialized = false;
static bool g_ShowMenu = true;
static bool g_GodMode = false;
static bool g_InfiniteAmmo = true;
static float g_SpeedMult = 1.0f;

typedef int32_t (*t_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*t_AInputQueue_finishEvent)(AInputQueue* queue, AInputEvent* event, int handled);
typedef EGLBoolean (*t_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

static t_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;
static t_AInputQueue_finishEvent orig_AInputQueue_finishEvent = nullptr;
static t_eglSwapBuffers orig_eglSwapBuffers = nullptr;

// Universal Input Hook: Hooks standard AInputQueue events used by Unity, Unreal, Cocos, Godot & C++ engines
int32_t my_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t res = orig_AInputQueue_getEvent(queue, outEvent);
    if (res != 0 || outEvent == nullptr || *outEvent == nullptr) {
        return res;
    }

    AInputEvent* event = *outEvent;
    int32_t type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        size_t pointerCount = AMotionEvent_getPointerCount(event);

        if (g_ImGuiInitialized) {
            ImGuiIO& io = ImGui::GetIO();

            if (pointerCount > 0) {
                float x = AMotionEvent_getX(event, 0);
                float y = AMotionEvent_getY(event, 0);
                io.AddMousePosEvent(x, y);
            }

            switch (actionMasked) {
                case AMOTION_EVENT_ACTION_DOWN:
                case AMOTION_EVENT_ACTION_POINTER_DOWN:
                    io.AddMouseButtonEvent(0, true);
                    break;
                case AMOTION_EVENT_ACTION_UP:
                case AMOTION_EVENT_ACTION_POINTER_UP:
                case AMOTION_EVENT_ACTION_CANCEL:
                    io.AddMouseButtonEvent(0, false);
                    break;
                default:
                    break;
            }

            // Universal touch consumption when ImGui requires mouse capture
            if (g_ShowMenu && io.WantCaptureMouse) {
                if (orig_AInputQueue_finishEvent) {
                    orig_AInputQueue_finishEvent(queue, event, 1);
                }
                return my_AInputQueue_getEvent(queue, outEvent);
            }
        }
    }

    return res;
}

// Universal EGL Swap Hook for ImGui Rendering
EGLBoolean my_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)width, (float)height);
            io.IniFilename = nullptr;

            ImGui::StyleColorsDark();
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(2.0f);

            ImGui_ImplOpenGL3_Init("#version 300 es");
            g_ImGuiInitialized = true;
            LOGI("ImGui Context initialized successfully: %dx%d", width, height);
        }
    }

    if (g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Screen Floating Toggle Button
        ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(140, 60), ImGuiCond_FirstUseEver);
        ImGui::Begin("Toggle", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        if (ImGui::Button(g_ShowMenu ? "Hide Menu" : "Open Menu", ImVec2(120, 40))) {
            g_ShowMenu = !g_ShowMenu;
        }
        ImGui::End();

        if (g_ShowMenu) {
            ImGui::SetNextWindowPos(ImVec2(200, 30), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(550, 380), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Universal ImGui Menu (Touch Fixed)", &g_ShowMenu)) {
                ImGui::Text("Universal Engine Support (Unity / Unreal / C++)");
                ImGui::Separator();
                ImGui::Checkbox("God Mode", &g_GodMode);
                ImGui::Checkbox("Infinite Ammo", &g_InfiniteAmmo);
                ImGui::SliderFloat("Speed Multiplier", &g_SpeedMult, 1.0f, 10.0f);
                ImGui::Separator();
                if (ImGui::Button("Close Menu", ImVec2(120, 40))) {
                    g_ShowMenu = false;
                }
            }
            ImGui::End();
        }

        ImGui::Render();

        GLint last_program, last_viewport[4], last_array_buffer;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        glGetIntegerv(GL_VIEWPORT, last_viewport);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glUseProgram(last_program);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
    }

    return orig_eglSwapBuffers(dpy, surface);
}

// Inline Hooking Engine
static bool perform_hook(void* target, void* replace, void** origin) {
    if (!target || !replace) return false;

    uintptr_t page_start = (uintptr_t)target & ~((uintptr_t)PAGE_SIZE - 1);
    if (mprotect((void*)page_start, PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        LOGE("Failed to set memory permissions for %p", target);
        return false;
    }

#if defined(__aarch64__)
    struct Trampoline64 {
        uint32_t ldr_x16;
        uint32_t br_x16;
        uint64_t addr;
    } __attribute__((packed));

    if (origin) {
        void* stub = mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        memcpy(stub, target, sizeof(Trampoline64));
        Trampoline64* back_jump = (Trampoline64*)((uintptr_t)stub + sizeof(Trampoline64));
        back_jump->ldr_x16 = 0x58000050;
        back_jump->br_x16 = 0xd61f0200;
        back_jump->addr = (uint64_t)target + sizeof(Trampoline64);
        *origin = stub;
    }

    Trampoline64* patch = (Trampoline64*)target;
    patch->ldr_x16 = 0x58000050;
    patch->br_x16 = 0xd61f0200;
    patch->addr = (uint64_t)replace;

    __builtin___clear_cache((char*)target, (char*)target + sizeof(Trampoline64));
    return true;
#elif defined(__arm__)
    struct Trampoline32 {
        uint32_t ldr_pc;
        uint32_t addr;
    } __attribute__((packed));

    if (origin) {
        void* stub = mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        memcpy(stub, target, sizeof(Trampoline32));
        Trampoline32* back_jump = (Trampoline32*)((uintptr_t)stub + sizeof(Trampoline32));
        back_jump->ldr_pc = 0xe51ff004;
        back_jump->addr = (uint32_t)target + sizeof(Trampoline32);
        *origin = stub;
    }

    Trampoline32* patch = (Trampoline32*)target;
    patch->ldr_pc = 0xe51ff004;
    patch->addr = (uint32_t)(uintptr_t)replace;

    __builtin___clear_cache((char*)target, (char*)target + sizeof(Trampoline32));
    return true;
#else
    if (origin) *origin = target;
    return true;
#endif
}

void* init_hooks_thread(void*) {
    sleep(2);

    void* egl_sym = dlsym(RTLD_DEFAULT, "eglSwapBuffers");
    if (egl_sym) {
        perform_hook(egl_sym, (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
        LOGI("Hooked eglSwapBuffers successfully at %p", egl_sym);
    }

    void* get_event_sym = dlsym(RTLD_DEFAULT, "AInputQueue_getEvent");
    if (get_event_sym) {
        perform_hook(get_event_sym, (void*)my_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        LOGI("Hooked AInputQueue_getEvent successfully at %p", get_event_sym);
    }

    void* finish_event_sym = dlsym(RTLD_DEFAULT, "AInputQueue_finishEvent");
    if (finish_event_sym) {
        orig_AInputQueue_finishEvent = (t_AInputQueue_finishEvent)finish_event_sym;
        LOGI("Resolved AInputQueue_finishEvent at %p", finish_event_sym);
    }

    return nullptr;
}

void setup_hooks() {
    pthread_t thread;
    pthread_create(&thread, nullptr, init_hooks_thread, nullptr);
}

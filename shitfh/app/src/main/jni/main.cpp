#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <android/log.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <chrono>
#include <string>

#include "zygisk.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ZygiskImGui", __VA_ARGS__)

// Custom Inline Hook Engine for ARM64 Target
#if defined(__aarch64__)
void inline_hook_arm64(void* target, void* hook, void** original) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    void* page_start = (void*)((uintptr_t)target & ~(page_size - 1));
    
    mprotect(page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    
    uint32_t* orig_code = (uint32_t*)target;
    
    void* trampoline = mmap(nullptr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    uint32_t* tramp_code = (uint32_t*)trampoline;
    
    // Copy first 4 instructions (16 bytes)
    tramp_code[0] = orig_code[0];
    tramp_code[1] = orig_code[1];
    tramp_code[2] = orig_code[2];
    tramp_code[3] = orig_code[3];
    
    // Jump back to target + 16
    tramp_code[4] = 0x58000050; // LDR X16, #8
    tramp_code[5] = 0xD61F0200; // BR X16
    uint64_t* tramp_dest = (uint64_t*)&tramp_code[6];
    *tramp_dest = (uint64_t)target + 16;
    
    *original = trampoline;
    
    // Overwrite target with jump to hook
    orig_code[0] = 0x58000050; // LDR X16, #8
    orig_code[1] = 0xD61F0200; // BR X16
    uint64_t* hook_dest = (uint64_t*)&orig_code[2];
    *hook_dest = (uint64_t)hook;
    
    __builtin___clear_cache((char*)target, (char*)target + 32);
    __builtin___clear_cache((char*)trampoline, (char*)trampoline + 64);
    
    mprotect(page_start, page_size * 2, PROT_READ | PROT_EXEC);
}
#endif

// Hook typedefs
typedef int (*AInputQueue_getEvent_t)(AInputQueue*, AInputEvent**);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue*, AInputEvent*, int);
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay, EGLSurface);

AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;
eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

bool g_Initialized = false;
int g_Width = 0;
int g_Height = 0;

// Universal Touch processing
bool handle_input_event(AInputEvent* event) {
    if (!ImGui::GetCurrentContext()) return false;
    
    ImGuiIO& io = ImGui::GetIO();
    int32_t type = AInputEvent_getType(event);
    
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        
        io.AddMousePosEvent(x, y);
        
        if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMouseButtonEvent(0, true);
        } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP) {
            io.AddMouseButtonEvent(0, false);
        }
        
        // Return true to consume input if ImGui wants mouse capture
        return io.WantCaptureMouse;
    }
    return false;
}

int my_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    while (true) {
        int res = orig_AInputQueue_getEvent(queue, outEvent);
        if (res < 0) return res;
        
        if (*outEvent != nullptr) {
            if (handle_input_event(*outEvent)) {
                // Finish event and notify system it's consumed, then loop to get the next event
                orig_AInputQueue_finishEvent(queue, *outEvent, 1);
                continue;
            }
        }
        return res;
    }
}

void my_AInputQueue_finishEvent(AInputQueue* queue, AInputEvent* event, int handled) {
    orig_AInputQueue_finishEvent(queue, event, handled);
}

void InitImGui(EGLDisplay display, EGLSurface surface) {
    eglQuerySurface(display, surface, EGL_WIDTH, &g_Width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &g_Height);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.DisplaySize = ImVec2((float)g_Width, (float)g_Height);
    ImGui::StyleColorsDark();
    
    ImGui_ImplOpenGL3_Init("#version 300 es");
    g_Initialized = true;
}

EGLBoolean my_eglSwapBuffers(EGLDisplay display, EGLSurface surface) {
    if (!g_Initialized) {
        InitImGui(display, surface);
    }
    
    int width = 0, height = 0;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    if (width != g_Width || height != g_Height) {
        g_Width = width;
        g_Height = height;
        ImGui::GetIO().DisplaySize = ImVec2((float)g_Width, (float)g_Height);
    }
    
    // Save critical OpenGL state
    GLint last_program, last_texture, last_fbo, last_active_texture;
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
    
    static auto last_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();
    float delta_time = std::chrono::duration<float>(current_time - last_time).count();
    last_time = current_time;
    ImGui::GetIO().DeltaTime = delta_time > 0.0f ? delta_time : (1.0f / 60.0f);
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    
    // Draw Universal Menu Overlay
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Universal Game Overlay (Zygisk)");
    ImGui::Text("Universal Input Hook: WORKING");
    ImGui::Text("Graphics Hook: GLES3/EGL SUCCESS");
    ImGui::Separator();
    static bool feature_1 = false;
    static bool feature_2 = false;
    ImGui::Checkbox("ImGui Input Block test", &feature_1);
    ImGui::Checkbox("Mock Hack Feature", &feature_2);
    ImGui::End();
    
    ImGui::Render();
    glViewport(0, 0, g_Width, g_Height);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Restore original OpenGL state
    glUseProgram(last_program);
    glActiveTexture(last_active_texture);
    glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
    
    return orig_eglSwapBuffers(display, surface);
}

void setup_hooks() {
#if defined(__aarch64__)
    LOGI("AArch64 environment identified. Starting inline hooking sequence.");
    
    void* libEGL = dlopen("libEGL.so", RTLD_LAZY);
    if (libEGL) {
        void* swapBuffers = dlsym(libEGL, "eglSwapBuffers");
        if (swapBuffers) {
            inline_hook_arm64(swapBuffers, (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("eglSwapBuffers Hooked!");
        }
    }
    
    void* libandroid = dlopen("libandroid.so", RTLD_LAZY);
    if (libandroid) {
        void* getEvent = dlsym(libandroid, "AInputQueue_getEvent");
        void* finishEvent = dlsym(libandroid, "AInputQueue_finishEvent");
        if (getEvent && finishEvent) {
            inline_hook_arm64(getEvent, (void*)my_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            inline_hook_arm64(finishEvent, (void*)my_AInputQueue_finishEvent, (void**)&orig_AInputQueue_finishEvent);
            LOGI("AInputQueue (Universal Input Handler) Hooked successfully!");
        }
    }
#else
    LOGI("Skipping hooks: 32-bit not supported.");
#endif
}

class ImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::ApiTable *api, jclass clazz) override {
        this->api = api;
        this->clazz = clazz;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        const char *process = api->getArgString(args->process_name);
        if (process) {
            std::string proc_name(process);
            
            // Read target configuration from static world-readable file
            char target[256] = {0};
            int fd = open("/data/local/tmp/imgui_target", O_RDONLY);
            if (fd >= 0) {
                read(fd, target, sizeof(target) - 1);
                close(fd);
                std::string target_str(target);
                target_str.erase(target_str.find_last_not_of(" \n\r\t") + 1);
                if (proc_name == target_str) {
                    enable_hook = true;
                }
            } else {
                // Fallback default target application
                if (proc_name == "com.epicgames.portal" || proc_name == "com.unity.game") {
                    enable_hook = true;
                }
            }
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (enable_hook) {
            LOGI("Target match found! Dispatching Hook Thread...");
            pthread_t thread;
            pthread_create(&thread, nullptr, (void* (*)(void*))install_hooks_thread, nullptr);
        }
    }

private:
    zygisk::ApiTable *api;
    jclass clazz;
    bool enable_hook = false;

    static void* install_hooks_thread(void*) {
        // Sleep for library dependencies load completion before patching
        sleep(3);
        setup_hooks();
        return nullptr;
    }
};

REGISTER_ZYGISK_MODULE(ImGuiModule)

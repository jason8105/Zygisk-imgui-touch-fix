#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <atomic>

#include "zygisk.hpp"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "UniversalZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#if defined(__LP64__)
#define ELF_R_SYM(info) ((info) >> 32)
#else
#define ELF_R_SYM(info) ELF32_R_SYM(info)
#endif

typedef int32_t (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef void (*AInputQueue_finishEvent_t)(AInputQueue* queue, AInputEvent* event, int handled);
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay dpy, EGLSurface surface);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputQueue_finishEvent_t orig_AInputQueue_finishEvent = nullptr;
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static std::atomic<bool> g_ImGuiInitialized{false};
static std::atomic<bool> g_MenuVisible{true};

// ---------------------------------------------------------------------------
// UNIVERSAL TOUCH HOOK IMPLEMENTATION
// ---------------------------------------------------------------------------
int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t res = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;
    if (res == 0 && outEvent && *outEvent) {
        AInputEvent* event = *outEvent;
        int32_t type = AInputEvent_getType(event);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            int32_t action = AMotionEvent_getAction(event);
            int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);

            if (g_ImGuiInitialized.load()) {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(x, y);

                if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
                    io.AddMouseButtonEvent(0, false);
                }

                // If ImGui wants mouse capture, consume touch and bypass engine
                if (io.WantCaptureMouse) {
                    if (orig_AInputQueue_finishEvent) {
                        orig_AInputQueue_finishEvent(queue, event, 1);
                    }
                    return hook_AInputQueue_getEvent(queue, outEvent);
                }
            }
        }
    }
    return res;
}

void hook_AInputQueue_finishEvent(AInputQueue* queue, AInputEvent* event, int handled) {
    if (orig_AInputQueue_finishEvent) {
        orig_AInputQueue_finishEvent(queue, event, handled);
    }
}

// ---------------------------------------------------------------------------
// EGL RENDERING LOOP & IMGUI INITIALIZATION
// ---------------------------------------------------------------------------
static void InitImGui(EGLDisplay dpy, EGLSurface surface) {
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (width <= 0 || height <= 0) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.ScaleAllSizes(2.2f);

    ImGui_ImplOpenGL3_Init("#version 300 es");
    g_ImGuiInitialized.store(true);
    LOGI("Universal ImGui initialized. Native screen size: %dx%d", width, height);
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized.load()) {
        InitImGui(dpy, surface);
    } else {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        if (width > 0 && height > 0) {
            ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        if (g_MenuVisible.load()) {
            ImGui::SetNextWindowSize(ImVec2(400, 280), ImGuiCond_FirstUseEver);
            ImGui::Begin("Universal ImGui Menu (Touch Fixed)", nullptr, ImGuiWindowFlags_NoCollapse);

            ImGui::Text("Engine: Universal Unity/Unreal/Native");
            ImGui::Separator();

            static bool feature_esp = true;
            static bool feature_aim = false;
            static float fov = 90.0f;

            ImGui::Checkbox("ESP Overlay", &feature_esp);
            ImGui::Checkbox("Aimbot Assistance", &feature_aim);
            ImGui::SliderFloat("FOV", &fov, 30.0f, 180.0f);

            if (ImGui::Button("Minimize Menu")) {
                g_MenuVisible.store(false);
            }

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

// ---------------------------------------------------------------------------
// PLT / GOT SYMBOL HOOKING
// ---------------------------------------------------------------------------
static void replace_got_entry(uintptr_t* got_entry, void* new_func, void** orig_func) {
    if (!got_entry || *got_entry == (uintptr_t)new_func) return;

    if (orig_func && *orig_func == nullptr) {
        *orig_func = (void*)*got_entry;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = (uintptr_t)got_entry & ~(page_size - 1);

    mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE);
    *got_entry = (uintptr_t)new_func;
    mprotect((void*)page_start, page_size, PROT_READ);
}

static void patch_elf_got(uintptr_t base_addr, const ElfW(Phdr)* phdr, int phnum, const char* symbol_name, void* hook_func, void** orig_func) {
    const ElfW(Dyn)* dyn = nullptr;
    for (int i = 0; i < phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = (const ElfW(Dyn)*)(base_addr + phdr[i].p_vaddr);
            break;
        }
    }
    if (!dyn) return;

    const ElfW(Sym)* symtab = nullptr;
    const char* strtab = nullptr;
    const ElfW(Rela)* rela = nullptr;
    size_t relasz = 0;
    const ElfW(Rel)* rel = nullptr;
    size_t relsz = 0;

    for (const ElfW(Dyn)* d = dyn; d->d_tag != DT_NULL; d++) {
        switch (d->d_tag) {
            case DT_SYMTAB: symtab = (const ElfW(Sym)*)(base_addr + d->d_un.d_ptr); break;
            case DT_STRTAB: strtab = (const char*)(base_addr + d->d_un.d_ptr); break;
            case DT_JMPREL:
                rela = (const ElfW(Rela)*)(base_addr + d->d_un.d_ptr);
                rel = (const ElfW(Rel)*)(base_addr + d->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                relasz = d->d_un.d_val / sizeof(ElfW(Rela));
                relsz = d->d_un.d_val / sizeof(ElfW(Rel));
                break;
        }
    }

    if (!symtab || !strtab) return;

    if (rela) {
        for (size_t i = 0; i < relasz; i++) {
            unsigned long sym_idx = ELF_R_SYM(rela[i].r_info);
            const char* name = strtab + symtab[sym_idx].st_name;
            if (strcmp(name, symbol_name) == 0) {
                uintptr_t* got_entry = (uintptr_t*)(base_addr + rela[i].r_offset);
                replace_got_entry(got_entry, hook_func, orig_func);
            }
        }
    }
    if (rel) {
        for (size_t i = 0; i < relsz; i++) {
            unsigned long sym_idx = ELF_R_SYM(rel[i].r_info);
            const char* name = strtab + symtab[sym_idx].st_name;
            if (strcmp(name, symbol_name) == 0) {
                uintptr_t* got_entry = (uintptr_t*)(base_addr + rel[i].r_offset);
                replace_got_entry(got_entry, hook_func, orig_func);
            }
        }
    }
}

static void apply_plt_hooks() {
    dl_iterate_phdr([](struct dl_phdr_info *info, size_t, void*) -> int {
        if (!info->dlpi_name || strlen(info->dlpi_name) == 0) return 0;

        patch_elf_got(info->dlpi_addr, info->dlpi_phdr, info->dlpi_phnum,
                      "AInputQueue_getEvent", (void*)hook_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
        patch_elf_got(info->dlpi_addr, info->dlpi_phdr, info->dlpi_phnum,
                      "AInputQueue_finishEvent", (void*)hook_AInputQueue_finishEvent, (void**)&orig_AInputQueue_finishEvent);
        patch_elf_got(info->dlpi_addr, info->dlpi_phdr, info->dlpi_phnum,
                      "eglSwapBuffers", (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);

        return 0;
    }, nullptr);
}

void* hook_worker_thread(void*) {
    void* libandroid = dlopen("libandroid.so", RTLD_NOW);
    if (libandroid) {
        orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(libandroid, "AInputQueue_getEvent");
        orig_AInputQueue_finishEvent = (AInputQueue_finishEvent_t)dlsym(libandroid, "AInputQueue_finishEvent");
    }

    void* libegl = dlopen("libEGL.so", RTLD_NOW);
    if (libegl) {
        orig_eglSwapBuffers = (eglSwapBuffers_t)dlsym(libegl, "eglSwapBuffers");
    }

    for (int i = 0; i < 20; i++) {
        apply_plt_hooks();
        sleep(1);
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// ZYGISK ENTRY POINT
// ---------------------------------------------------------------------------
class UniversalImGuiModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
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

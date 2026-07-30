#include <dobby.h>
#include <imgui.h>
#include <jni.h>

// Hook definition for Unity's input injection
typedef void (*NativeInjectEvent)(int, float, float);
NativeInjectEvent orig_inject = nullptr;

void hooked_inject(int type, float x, float y) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);
    
    // Consume if ImGui wants input
    if (io.WantCaptureMouse) return;

    orig_inject(type, x, y);
}

void setup_hooks() {
    void* handle = dlopen("libunity.so", RTLD_NOW);
    if (handle) {
        void* addr = dlsym(handle, "Unity_nativeInjectEvent");
        if (addr) {
            DobbyHook(addr, (void*)hooked_inject, (void**)&orig_inject);
        }
    }
}

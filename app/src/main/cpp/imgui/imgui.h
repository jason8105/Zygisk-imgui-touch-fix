#ifndef IMGUI_H_
#define IMGUI_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ImGuiContext ImGuiContext;
typedef struct ImDrawData ImDrawData;

typedef struct ImVec2 {
    float x, y;
} ImVec2;

typedef struct ImVec4 {
    float x, y, z, w;
} ImVec4;

typedef struct ImGuiIO {
    ImVec2 DisplaySize;
    float DeltaTime;
    bool WantCaptureMouse;
    bool WantCaptureKeyboard;
    void (*AddMousePosEvent)(float x, float y);
    void (*AddMouseButtonEvent)(int button, bool down);
} ImGuiIO;

#ifdef __cplusplus
} // extern "C"
#endif

// C++ Helper API definitions
#ifdef __cplusplus
namespace ImGui {
    IMGUI_IMPL_API ImGuiContext* CreateContext(ImGuiContext* shared_context = NULL);
    IMGUI_IMPL_API void DestroyContext(ImGuiContext* context = NULL);
    IMGUI_IMPL_API ImGuiIO& GetIO();
    IMGUI_IMPL_API void NewFrame();
    IMGUI_IMPL_API void Render();
    IMGUI_IMPL_API ImDrawData* GetDrawData();
    IMGUI_IMPL_API void StyleColorsDark(ImGuiStyle* dst = NULL);
    IMGUI_IMPL_API bool Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
    IMGUI_IMPL_API void End();
    IMGUI_IMPL_API bool Button(const char* label, const ImVec2& size = ImVec2(0,0));
    IMGUI_IMPL_API void Text(const char* fmt, ...);
}
#endif

#endif // IMGUI_H_

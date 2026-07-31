#include <android/input.h>
#include <android/keycodes.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

static bool g_ImGuiInitialized = false;
static bool g_ShowMenu = true;

int32_t (*orig_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent) = nullptr;
void (*orig_AInputQueue_finishEvent)(AInputQueue* queue, AInputEvent* event, int handled) = nullptr;
EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface) = nullptr;

// UNIVERSAL TOUCH HOOK
int32_t hook_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t result = orig_AInputQueue_getEvent ? orig_AInputQueue_getEvent(queue, outEvent) : -1;
    if (result < 0 || !outEvent || !*outEvent) {
        return result;
    }

    int32_t eventType = AInputEvent_getType(*outEvent);
    if (eventType == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(*outEvent);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        float x = AMotionEvent_getX(*outEvent, pointerIndex);
        float y = AMotionEvent_getY(*outEvent, pointerIndex);

        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);

        if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            io.AddMouseButtonEvent(0, true);
        } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
            io.AddMouseButtonEvent(0, false);
        }

        // Consume touch event if ImGui captures input
        if (g_ShowMenu && io.WantCaptureMouse) {
            if (orig_AInputQueue_finishEvent) {
                orig_AInputQueue_finishEvent(queue, *outEvent, 1);
            }
            return hook_AInputQueue_getEvent(queue, outEvent);
        }
    }

    return result;
}

void hook_AInputQueue_finishEvent(AInputQueue* queue, AInputEvent* event, int handled) {
    if (orig_AInputQueue_finishEvent) {
        orig_AInputQueue_finishEvent(queue, event, handled);
    }
}

// UNIVERSAL RENDER HOOK
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (width > 0 && height > 0) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)width, (float)height);

            ImGui::StyleColorsDark();
            ImGui_ImplOpenGL3_Init("#version 300 es");
            g_ImGuiInitialized = true;
        }
    } else {
        EGLint width = 0, height = 0;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);
    }

    if (g_ImGuiInitialized) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        if (g_ShowMenu) {
            ImGui::SetNextWindowSize(ImVec2(380, 260), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Universal Zygisk Menu", &g_ShowMenu)) {
                ImGui::Text("Universal Touch & Engine Fix Active");
                ImGui::Separator();

                static bool feature1 = false;
                static bool feature2 = false;
                static float slider_val = 50.0f;

                ImGui::Checkbox("Feature 1", &feature1);
                ImGui::Checkbox("Feature 2", &feature2);
                ImGui::SliderFloat("Value", &slider_val, 0.0f, 100.0f);
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers ? orig_eglSwapBuffers(dpy, surface) : EGL_TRUE;
}

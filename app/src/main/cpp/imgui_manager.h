#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace ImGuiManager {
    void Init();
    void Render(EGLDisplay dpy, EGLSurface surface);
}

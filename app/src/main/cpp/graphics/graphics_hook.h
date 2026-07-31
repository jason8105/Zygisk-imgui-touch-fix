#pragma once

#include <EGL/egl.h>

namespace GraphicsHook {
    void InstallHooks();
    EGLBoolean Hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
}

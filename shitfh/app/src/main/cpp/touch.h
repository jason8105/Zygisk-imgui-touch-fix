#pragma once

#include <android/input.h>
#include <EGL/egl.h>

namespace TouchHandler {
    void InitHooks();
    bool ProcessInputEvent(AInputEvent* event);
    void OnEglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
}

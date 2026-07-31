#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <jni.h>

namespace ImGuiManager {
    void Init(JNIEnv* env);
    void OnEglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
}

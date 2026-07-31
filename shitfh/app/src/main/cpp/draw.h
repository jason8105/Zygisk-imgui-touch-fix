#ifndef DRAW_H
#define DRAW_H

#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace GraphicsHook {
    void Init();
    EGLBoolean SwapBuffersHook(EGLDisplay dpy, EGLSurface surface);
}

#endif // DRAW_H

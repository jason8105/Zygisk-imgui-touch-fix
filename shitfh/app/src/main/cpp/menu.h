#ifndef MENU_H
#define MENU_H

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);

#ifdef __cplusplus
}
#endif

#endif // MENU_H

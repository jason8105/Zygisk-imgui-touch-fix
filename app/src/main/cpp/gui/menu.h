#ifndef MENU_H
#define MENU_H

#include <EGL/egl.h>

class Menu {
public:
    static void OnSwapBuffers(EGLDisplay dpy, EGLSurface surface);
    static bool HandleTouch(int action, float x, float y);
    static void Render();
};

#endif // MENU_H

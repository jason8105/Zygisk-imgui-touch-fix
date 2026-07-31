#ifndef GUI_H
#define GUI_H

#include <EGL/egl.h>

namespace MenuGUI {
    void Init();
    void Render();
    bool IsVisible();
    void ToggleVisible();
}

#endif // GUI_H

#ifndef HOOKS_H
#define HOOKS_H

#include <android/input.h>
#include <android/native_window.h>

void install_universal_hooks();
bool handle_input_event(AInputEvent* event);

#endif

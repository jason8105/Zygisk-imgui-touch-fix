#ifndef INPUT_HOOK_H
#define INPUT_HOOK_H

#include <android/input.h>

void install_input_hooks();
bool handle_input_event(AInputEvent* event);

#endif

#pragma once

#include <stddef.h>

namespace Hook {
    void InitHooks(void* swapBuffersHook, void** origSwapBuffers);
    void PLTHook(const char* libName, const char* symName, void* newFunc, void** oldFunc);
}

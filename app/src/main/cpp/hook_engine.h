#pragma once

#include <cstdint>
#include <dlfcn.h>

namespace HookEngine {
    bool Hook(void *target, void *replace, void **origin);
    bool HookSymbol(const char *library_name, const char *symbol_name, void *replace, void **origin);
}

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace HookUtils {
    bool HookSymbol(const char* libName, const char* symbolName, void* hookFunc, void** origFunc);
    void* GetModuleBase(const char* moduleName);
}

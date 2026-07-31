#pragma once
#include <stddef.h>

namespace HookEngine {
    int Hook(void* symbol_addr, void* replace_addr, void** orig_addr);
    int HookSymbol(const char* lib_name, const char* symbol_name, void* replace_addr, void** orig_addr);
}

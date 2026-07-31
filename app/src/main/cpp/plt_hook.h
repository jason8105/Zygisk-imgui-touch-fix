#pragma once

#include <stddef.h>
#include <stdint.h>

namespace PltHook {
    bool HookSymbol(const char* lib_name, const char* symbol_name, void* new_func, void** old_func);
    bool HookAll(const char* symbol_name, void* new_func, void** old_func);
}

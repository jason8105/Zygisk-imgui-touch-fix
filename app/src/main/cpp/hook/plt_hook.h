#pragma once
#include <stddef.h>

namespace PltHook {
    bool HookSymbol(const char* library_name, const char* symbol_name, void* new_func, void** old_func);
    bool HookAllLoaded(const char* symbol_name, void* new_func, void** old_func);
}

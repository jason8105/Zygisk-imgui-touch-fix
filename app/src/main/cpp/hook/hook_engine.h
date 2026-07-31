#pragma once

#include <stdint.h>
#include <stddef.h>

namespace HookEngine {
    void PltHookModule(uintptr_t base, const char* symbol_name, void* new_func, void** old_func);
    void PltHookAllModules(const char* symbol_name, void* new_func, void** old_func);
}

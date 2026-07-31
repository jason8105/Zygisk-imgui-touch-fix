#pragma once

#include <stdint.h>
#include <stddef.h>

namespace plt_hook {
    void hook_all_modules(const char* symbol_name, void* new_func, void** old_func);
}

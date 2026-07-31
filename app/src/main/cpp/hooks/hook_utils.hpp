#pragma once

namespace HookUtils {
    bool HookSymbol(const char* lib_pattern, const char* symbol_name, void* new_func, void** old_func);
}

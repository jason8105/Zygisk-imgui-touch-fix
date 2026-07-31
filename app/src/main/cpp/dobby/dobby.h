#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int hook_symbol(const char* lib_name, const char* symbol_name, void* new_func, void** old_func);

#ifdef __cplusplus
}
#endif

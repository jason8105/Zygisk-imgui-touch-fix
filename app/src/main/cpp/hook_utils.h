#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void* find_library_symbol(const char* lib_name, const char* symbol_name);
bool plt_hook(const char* lib_name, const char* symbol_name, void* replace_fn, void** orig_fn);

#ifdef __cplusplus
}
#endif

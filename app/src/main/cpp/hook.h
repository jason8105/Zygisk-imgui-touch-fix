#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool plt_hook_symbol(const char* target_library, const char* symbol_name, void* hook_func, void** orig_func);

#ifdef __cplusplus
}
#endif

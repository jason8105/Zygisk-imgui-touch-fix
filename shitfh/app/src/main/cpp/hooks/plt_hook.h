#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* plt_hook_symbol(const char* lib_name, const char* symbol_name, void* hook_func);

#ifdef __cplusplus
}
#endif

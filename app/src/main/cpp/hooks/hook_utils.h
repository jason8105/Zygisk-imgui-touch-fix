#pragma once
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool hook_plt(const char *module_pattern, const char *symbol_name, void *hook_func, void **orig_func);

#ifdef __cplusplus
}
#endif

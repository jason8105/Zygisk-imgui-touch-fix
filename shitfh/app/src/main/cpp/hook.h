#ifndef HOOK_H
#define HOOK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void hook_plt_all(const char* symbol_name, void* new_func, void** old_func);

#ifdef __cplusplus
}
#endif

#endif // HOOK_H

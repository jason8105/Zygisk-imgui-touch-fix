#ifndef HOOK_H
#define HOOK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool HookFunction(void* target, void* replace, void** orig);
bool HookSymbol(const char* library_name, const char* symbol_name, void* replace, void** orig);

#ifdef __cplusplus
}
#endif

#endif // HOOK_H

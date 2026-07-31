#ifndef DOBBY_H
#define DOBBY_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *dobby_dummy_func_t;

int DobbyHook(void *function_address, dobby_dummy_func_t replace_func, dobby_dummy_func_t *origin_func);

#ifdef __cplusplus
}
#endif

#endif // DOBBY_H

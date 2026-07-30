#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

int DobbyHook(void *address, void *replace, void **backup);
int DobbyDestroy(void *address);

#if defined(__cplusplus)
}
#endif

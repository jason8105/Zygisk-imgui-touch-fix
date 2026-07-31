#include "hook_utils.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>

#define LOG_TAG "ZygiskHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#if defined(__aarch64__)
int DobbyHook(void *function_address, void *replace_call, void **origin_call) {
    if (!function_address || !replace_call) return -1;

    size_t pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = (uintptr_t)function_address & ~(pageSize - 1);

    uint32_t *trampoline = (uint32_t*)mmap(NULL, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (trampoline == MAP_FAILED) return -1;

    memcpy(trampoline, function_address, 16);

    uintptr_t returnAddr = (uintptr_t)function_address + 16;
    trampoline[4] = 0x58000050; // LDR X16, #8
    trampoline[5] = 0xd61f0200; // BR X16
    memcpy(&trampoline[6], &returnAddr, sizeof(void*));

    if (origin_call) {
        *origin_call = (void*)trampoline;
    }

    mprotect((void*)pageStart, pageSize * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

    uint32_t *target = (uint32_t*)function_address;
    uintptr_t hookAddr = (uintptr_t)replace_call;
    target[0] = 0x58000050; // LDR X16, #8
    target[1] = 0xd61f0200; // BR X16
    memcpy(&target[2], &hookAddr, sizeof(void*));

    __builtin___clear_cache((char*)function_address, (char*)function_address + 16);
    __builtin___clear_cache((char*)trampoline, (char*)trampoline + 32);

    mprotect((void*)pageStart, pageSize * 2, PROT_READ | PROT_EXEC);

    return 0;
}
#elif defined(__arm__)
int DobbyHook(void *function_address, void *replace_call, void **origin_call) {
    if (!function_address || !replace_call) return -1;

    size_t pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = (uintptr_t)function_address & ~(pageSize - 1);

    uint32_t *trampoline = (uint32_t*)mmap(NULL, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (trampoline == MAP_FAILED) return -1;

    memcpy(trampoline, function_address, 8);
    uintptr_t returnAddr = (uintptr_t)function_address + 8;
    trampoline[2] = 0xe51ff004; // LDR PC, [PC, #-4]
    trampoline[3] = (uint32_t)returnAddr;

    if (origin_call) {
        *origin_call = (void*)trampoline;
    }

    mprotect((void*)pageStart, pageSize * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

    uint32_t *target = (uint32_t*)function_address;
    target[0] = 0xe51ff004; // LDR PC, [PC, #-4]
    target[1] = (uint32_t)replace_call;

    __builtin___clear_cache((char*)function_address, (char*)function_address + 8);
    __builtin___clear_cache((char*)trampoline, (char*)trampoline + 16);

    mprotect((void*)pageStart, pageSize * 2, PROT_READ | PROT_EXEC);

    return 0;
}
#else
int DobbyHook(void *function_address, void *replace_call, void **origin_call) {
    return -1;
}
#endif

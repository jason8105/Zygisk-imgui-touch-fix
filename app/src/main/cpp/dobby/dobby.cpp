#include "dobby.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <android/log.h>

#define LOG_TAG "DobbyHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" int DobbyHook(void *function_address, void *replace_call, void **origin_call) {
    if (!function_address || !replace_call) return -1;

    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t addr = (uintptr_t)function_address;
    uintptr_t page_start = addr & ~(page_size - 1);

    void* trampoline_mem = mmap(nullptr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

#if defined(__aarch64__)
    size_t patch_size = 16;
    if (origin_call && trampoline_mem != MAP_FAILED) {
        memcpy(trampoline_mem, function_address, patch_size);
        uintptr_t return_addr = addr + patch_size;
        uint32_t jump_back[] = {
            0x58000050, // ldr x16, #8
            0xd61f0200, // br x16
            (uint32_t)(return_addr),
            (uint32_t)(return_addr >> 32)
        };
        memcpy((char*)trampoline_mem + patch_size, jump_back, sizeof(jump_back));
        *origin_call = trampoline_mem;
    }

    if (mprotect((void *)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
        uint32_t patch[] = {
            0x58000050, // ldr x16, #8
            0xd61f0200, // br x16
            (uint32_t)(uintptr_t)replace_call,
            (uint32_t)((uintptr_t)replace_call >> 32)
        };
        memcpy(function_address, patch, sizeof(patch));
        mprotect((void *)page_start, page_size * 2, PROT_READ | PROT_EXEC);
        __builtin___clear_cache((char*)function_address, (char*)function_address + patch_size);
    }
#elif defined(__arm__)
    size_t patch_size = 8;
    if (origin_call && trampoline_mem != MAP_FAILED) {
        memcpy(trampoline_mem, function_address, patch_size);
        uintptr_t return_addr = addr + patch_size;
        uint32_t jump_back[] = {
            0xe51ff004, // ldr pc, [pc, #-4]
            (uint32_t)(return_addr)
        };
        memcpy((char*)trampoline_mem + patch_size, jump_back, sizeof(jump_back));
        *origin_call = trampoline_mem;
    }

    if (mprotect((void *)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
        uint32_t patch[] = {
            0xe51ff004, // ldr pc, [pc, #-4]
            (uint32_t)(uintptr_t)replace_call
        };
        memcpy(function_address, patch, sizeof(patch));
        mprotect((void *)page_start, page_size * 2, PROT_READ | PROT_EXEC);
        __builtin___clear_cache((char*)function_address, (char*)function_address + patch_size);
    }
#endif

    return 0;
}

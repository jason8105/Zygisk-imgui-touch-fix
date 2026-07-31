#include "dobby.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define PAGE_MASK ~(PAGE_SIZE - 1)

static void unprotect_page(void *addr, size_t len) {
    uintptr_t start = (uintptr_t)addr & PAGE_MASK;
    uintptr_t end = ((uintptr_t)addr + len + PAGE_SIZE - 1) & PAGE_MASK;
    mprotect((void *)start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC);
}

int DobbyHook(void *function_address, void *replace_call, void **origin_call) {
    if (!function_address || !replace_call) return -1;

    void *trampoline = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (trampoline == MAP_FAILED) return -1;

#if defined(__aarch64__)
    uint8_t *src = (uint8_t *)function_address;
    uint8_t *tramp = (uint8_t *)trampoline;

    memcpy(tramp, src, 16);
    uintptr_t return_addr = (uintptr_t)src + 16;
    uint32_t tramp_jump[2] = { 0x58000050, 0xd61f0200 }; // ldr x16, #8 ; br x16
    memcpy(tramp + 16, tramp_jump, 8);
    memcpy(tramp + 24, &return_addr, 8);

    if (origin_call) *origin_call = trampoline;

    unprotect_page(function_address, 16);
    uint32_t hook_code[2] = { 0x58000050, 0xd61f0200 };
    uintptr_t target_addr = (uintptr_t)replace_call;

    memcpy(src, hook_code, 8);
    memcpy(src + 8, &target_addr, 8);

    __builtin___clear_cache((char *)src, (char *)src + 16);
    __builtin___clear_cache((char *)tramp, (char *)tramp + 32);
    return 0;

#elif defined(__arm__)
    uint8_t *src = (uint8_t *)function_address;
    uint8_t *tramp = (uint8_t *)trampoline;

    memcpy(tramp, src, 8);
    uintptr_t return_addr = (uintptr_t)src + 8;
    uint32_t tramp_jump = 0xe51ff004; // ldr pc, [pc, #-4]
    memcpy(tramp + 8, &tramp_jump, 4);
    memcpy(tramp + 12, &return_addr, 4);

    if (origin_call) *origin_call = trampoline;

    unprotect_page(function_address, 8);
    uint32_t hook_code = 0xe51ff004;
    uintptr_t target_addr = (uintptr_t)replace_call;

    memcpy(src, &hook_code, 4);
    memcpy(src + 4, &target_addr, 4);

    __builtin___clear_cache((char *)src, (char *)src + 8);
    __builtin___clear_cache((char *)tramp, (char *)tramp + 16);
    return 0;

#elif defined(__x86_64__)
    uint8_t *src = (uint8_t *)function_address;
    uint8_t *tramp = (uint8_t *)trampoline;

    memcpy(tramp, src, 14);
    uintptr_t return_addr = (uintptr_t)src + 14;
    uint8_t tramp_jump[6] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    memcpy(tramp + 14, tramp_jump, 6);
    memcpy(tramp + 20, &return_addr, 8);

    if (origin_call) *origin_call = trampoline;

    unprotect_page(function_address, 14);
    uint8_t hook_code[6] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
    uintptr_t target_addr = (uintptr_t)replace_call;

    memcpy(src, hook_code, 6);
    memcpy(src + 6, &target_addr, 8);
    return 0;

#else
    uint8_t *src = (uint8_t *)function_address;
    uint8_t *tramp = (uint8_t *)trampoline;

    memcpy(tramp, src, 5);
    uintptr_t return_addr = (uintptr_t)src + 5;
    tramp[5] = 0xE9;
    int32_t rel_tramp = (int32_t)(return_addr - ((uintptr_t)tramp + 10));
    memcpy(tramp + 6, &rel_tramp, 4);

    if (origin_call) *origin_call = trampoline;

    unprotect_page(function_address, 5);
    src[0] = 0xE9;
    int32_t rel_hook = (int32_t)((uintptr_t)replace_call - ((uintptr_t)src + 5));
    memcpy(src + 1, &rel_hook, 4);
    return 0;
#endif
}

#include "dobby.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

extern "C" int DobbyHook(void *function_address, dobby_dummy_func_t replace_func, dobby_dummy_func_t *origin_func) {
    if (!function_address || !replace_func) return -1;

#if defined(__aarch64__)
    uint32_t *target = (uint32_t *)function_address;
    long page_size = sysconf(_SC_PAGESIZE);
    void *page_start = (void *)((uintptr_t)target & ~(page_size - 1));

    if (mprotect(page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return -1;
    }

    if (origin_func) {
        void *trampoline = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (trampoline != MAP_FAILED) {
            memcpy(trampoline, target, 16);
            uint32_t *t_code = (uint32_t *)((uint8_t *)trampoline + 16);
            uint64_t ret_addr = (uint64_t)target + 16;
            t_code[0] = 0x58000051; // LDR X17, #8
            t_code[1] = 0xd61f0220; // BR X17
            memcpy(&t_code[2], &ret_addr, sizeof(ret_addr));
            *origin_func = trampoline;
        }
    }

    uint64_t jump_dest = (uint64_t)replace_func;
    target[0] = 0x58000051; // LDR X17, #8
    target[1] = 0xd61f0220; // BR X17
    memcpy(&target[2], &jump_dest, sizeof(jump_dest));

    mprotect(page_start, page_size * 2, PROT_READ | PROT_EXEC);
    return 0;
#else
    if (origin_func) *origin_func = function_address;
    return 0;
#endif
}

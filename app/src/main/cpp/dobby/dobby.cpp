#include "dobby.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

extern "C" {

int DobbyHook(void *function_address, dobby_dummy_func_t replace_func, dobby_dummy_func_t *origin_func) {
    if (!function_address || !replace_func) return -1;

    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t addr = (uintptr_t)function_address;
    uintptr_t page_start = addr & ~(page_size - 1);

    mprotect((void *)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

#if defined(__aarch64__)
    if (origin_func) {
        uint32_t *tramp = (uint32_t *)mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        memcpy(tramp, (void *)addr, 16);
        
        uint64_t return_addr = addr + 16;
        tramp[4] = 0x58000051; // ldr x17, #8
        tramp[5] = 0xd61f0220; // br x17
        memcpy(&tramp[6], &return_addr, sizeof(return_addr));
        
        *origin_func = (dobby_dummy_func_t)tramp;
    }

    uint32_t *code = (uint32_t *)addr;
    uint64_t target = (uint64_t)replace_func;
    code[0] = 0x58000051; // ldr x17, #8
    code[1] = 0xd61f0220; // br x17
    memcpy(&code[2], &target, sizeof(target));

#elif defined(__arm__)
    if (origin_func) {
        uint32_t *tramp = (uint32_t *)mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        memcpy(tramp, (void *)addr, 8);
        
        uint32_t return_addr = addr + 8;
        tramp[2] = 0xe51ff004; // ldr pc, [pc, #-4]
        tramp[3] = return_addr;
        
        *origin_func = (dobby_dummy_func_t)tramp;
    }

    uint32_t *code = (uint32_t *)addr;
    uint32_t target = (uint32_t)replace_func;
    code[0] = 0xe51ff004; // ldr pc, [pc, #-4]
    code[1] = target;
#endif

    mprotect((void *)page_start, page_size * 2, PROT_READ | PROT_EXEC);
    return 0;
}

}

#include "dobby.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

extern "C" int DobbyHook(void *function_address, void *replace_call, void **origin_call) {
    if (!function_address || !replace_call) return -1;

    size_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = (uintptr_t)function_address & ~(page_size - 1);

    if (mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return -1;
    }

#if defined(__aarch64__)
    uint32_t *target = (uint32_t *)function_address;
    if (origin_call) {
        void *stub = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (stub != MAP_FAILED) {
            memcpy(stub, function_address, 16);
            uint32_t *stub_code = (uint32_t *)((uintptr_t)stub + 16);
            uintptr_t return_addr = (uintptr_t)function_address + 16;
            
            stub_code[0] = 0xd503201f;
            stub_code[1] = 0x58000050;
            stub_code[2] = 0xd61f0200;
            *(uintptr_t *)&stub_code[3] = return_addr;

            *origin_call = stub;
        }
    }

    target[0] = 0x58000050;
    target[1] = 0xd61f0200;
    *(uintptr_t *)&target[2] = (uintptr_t)replace_call;

#elif defined(__arm__)
    uint32_t *target = (uint32_t *)function_address;
    if (origin_call) {
        void *stub = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (stub != MAP_FAILED) {
            memcpy(stub, function_address, 8);
            uint32_t *stub_code = (uint32_t *)((uintptr_t)stub + 8);
            uintptr_t return_addr = (uintptr_t)function_address + 8;

            stub_code[0] = 0xe59ff000;
            *(uintptr_t *)&stub_code[1] = return_addr;

            *origin_call = stub;
        }
    }

    target[0] = 0xe59ff000;
    *(uintptr_t *)&target[1] = (uintptr_t)replace_call;
#endif

    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_EXEC);
    return 0;
}

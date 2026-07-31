#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <cstring>
#include <android/log.h>

#define LOG_TAG "ZygiskHookEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" void HookSymbol(void* target, void* replace, void** origin) {
    if (!target || !replace) return;

    if (origin) {
        *origin = target;
    }

#if defined(__aarch64__)
    uint32_t trampoline[] = {
        0x58000050, // ldr x16, #8
        0xd61f0200, // br x16
        0x00000000,
        0x00000000
    };
    uint64_t addr = (uint64_t)replace;
    memcpy(&trampoline[2], &addr, sizeof(addr));

    uintptr_t page_start = (uintptr_t)target & ~((uintptr_t)sysconf(_SC_PAGESIZE) - 1);
    mprotect((void*)page_start, sysconf(_SC_PAGESIZE) * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(target, trampoline, sizeof(trampoline));
    mprotect((void*)page_start, sysconf(_SC_PAGESIZE) * 2, PROT_READ | PROT_EXEC);
    __builtin___clear_cache((char*)target, (char*)target + sizeof(trampoline));

#elif defined(__arm__)
    uint32_t trampoline[] = {
        0xe51ff004, // ldr pc, [pc, #-4]
        (uint32_t)replace
    };

    uintptr_t page_start = (uintptr_t)target & ~((uintptr_t)sysconf(_SC_PAGESIZE) - 1);
    mprotect((void*)page_start, sysconf(_SC_PAGESIZE) * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(target, trampoline, sizeof(trampoline));
    mprotect((void*)page_start, sysconf(_SC_PAGESIZE) * 2, PROT_READ | PROT_EXEC);
    __builtin___clear_cache((char*)target, (char*)target + sizeof(trampoline));
#endif

    LOGI("HookSymbol successfully applied at %p -> %p", target, replace);
}

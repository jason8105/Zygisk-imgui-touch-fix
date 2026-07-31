#include "hook.h"
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <android/log.h>

#define LOG_TAG "ZygiskHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static bool Unprotect(uintptr_t addr, size_t size) {
    long page_size = sysconf(_SC_PAGESIZE);
    uintptr_t start = addr & ~(page_size - 1);
    uintptr_t end = (addr + size + page_size - 1) & ~(page_size - 1);
    return mprotect((void*)start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

#if defined(__aarch64__)
struct TrampolineARM64 {
    uint32_t ldr_x16; // ldr x16, #8
    uint32_t br_x16;  // br x16
    uint64_t target;
};
#elif defined(__arm__)
struct TrampolineARM32 {
    uint32_t ldr_pc;  // ldr pc, [pc, #-4]
    uint32_t target;
};
#endif

bool HookFunction(void* target, void* replace, void** orig) {
    if (!target || !replace) return false;

    if (orig) {
        *orig = target;
    }

#if defined(__aarch64__)
    if (!Unprotect((uintptr_t)target, sizeof(TrampolineARM64))) {
        LOGE("Failed to unprotect memory at %p", target);
        return false;
    }
    TrampolineARM64* tramp = (TrampolineARM64*)target;
    tramp->ldr_x16 = 0x58000050; // ldr x16, #8
    tramp->br_x16  = 0xd61f0200; // br x16
    tramp->target  = (uint64_t)replace;
    __builtin___clear_cache((char*)target, (char*)target + sizeof(TrampolineARM64));
    return true;
#elif defined(__arm__)
    uintptr_t target_addr = (uintptr_t)target & ~1;
    if (!Unprotect(target_addr, sizeof(TrampolineARM32))) {
        LOGE("Failed to unprotect memory at %p", target);
        return false;
    }
    TrampolineARM32* tramp = (TrampolineARM32*)target_addr;
    tramp->ldr_pc = 0xe59ff000;
    tramp->target = (uint32_t)replace;
    __builtin___clear_cache((char*)target_addr, (char*)target_addr + sizeof(TrampolineARM32));
    return true;
#else
    return false;
#endif
}

bool HookSymbol(const char* library_name, const char* symbol_name, void* replace, void** orig) {
    void* handle = dlopen(library_name, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        handle = RTLD_DEFAULT;
    }
    void* sym = dlsym(handle, symbol_name);
    if (!sym) {
        LOGE("Symbol %s not found in %s", symbol_name, library_name ? library_name : "default");
        return false;
    }
    return HookFunction(sym, replace, orig);
}

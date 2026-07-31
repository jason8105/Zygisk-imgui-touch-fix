#include "hook_engine.h"
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cstring>
#include <android/log.h>

#define LOG_TAG "ZygiskHookEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace HookEngine {

struct HookTarget {
    void* symbol_addr;
    void* replace_addr;
    void** orig_addr;
    bool found;
};

static int phdr_callback(struct dl_phdr_info *info, size_t size, void *data) {
    HookTarget* target = static_cast<HookTarget*>(data);
    if (!info || !target || !target->symbol_addr) return 0;

    ElfW(Addr) base = info->dlpi_addr;
    const ElfW(Phdr) *phdr = info->dlpi_phdr;

    for (int i = 0; i < info->dlpi_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            ElfW(Dyn) *dyn = reinterpret_cast<ElfW(Dyn)*>(base + phdr[i].p_vaddr);
            ElfW(Addr) *pltgot = nullptr;

            for (ElfW(Dyn) *d = dyn; d->d_tag != DT_NULL; d++) {
                if (d->d_tag == DT_PLTGOT) {
                    pltgot = reinterpret_cast<ElfW(Addr)*>(d->d_un.d_ptr);
                    if (reinterpret_cast<ElfW(Addr)>(pltgot) < base) {
                        pltgot = reinterpret_cast<ElfW(Addr)*>(base + reinterpret_cast<ElfW(Addr)>(pltgot));
                    }
                    break;
                }
            }

            if (pltgot) {
                for (int j = 0; j < 1024; j++) {
                    if (pltgot[j] == reinterpret_cast<ElfW(Addr)>(target->symbol_addr)) {
                        uintptr_t page_start = reinterpret_cast<uintptr_t>(&pltgot[j]) & ~((uintptr_t)sysconf(_SC_PAGESIZE) - 1);
                        size_t page_size = sysconf(_SC_PAGESIZE);

                        mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_WRITE);
                        if (target->orig_addr && *target->orig_addr == nullptr) {
                            *target->orig_addr = target->symbol_addr;
                        }
                        pltgot[j] = reinterpret_cast<ElfW(Addr)>(target->replace_addr);
                        mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_EXEC);
                        target->found = true;
                    }
                }
            }
        }
    }
    return 0;
}

int Hook(void* symbol_addr, void* replace_addr, void** orig_addr) {
    if (!symbol_addr || !replace_addr) return -1;
    if (orig_addr) *orig_addr = symbol_addr;

    HookTarget target = { symbol_addr, replace_addr, orig_addr, false };
    dl_iterate_phdr(phdr_callback, &target);
    return 0;
}

int HookSymbol(const char* lib_name, const char* symbol_name, void* replace_addr, void** orig_addr) {
    void* handle = dlopen(lib_name, RTLD_LAZY);
    if (!handle) handle = RTLD_DEFAULT;
    void* sym = dlsym(handle, symbol_name);
    if (!sym) return -1;
    return Hook(sym, replace_addr, orig_addr);
}

} // namespace HookEngine

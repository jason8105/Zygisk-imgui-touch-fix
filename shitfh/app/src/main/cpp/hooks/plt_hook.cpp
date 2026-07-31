#include "plt_hook.h"
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <string.h>
#include <android/log.h>
#include <unistd.h>

#define LOG_TAG "PLTHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct HookContext {
    const char* lib_name;
    const char* symbol_name;
    void* hook_func;
    void* orig_func;
};

static int dl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data) {
    auto ctx = static_cast<HookContext*>(data);
    if (!info->dlpi_name) return 0;

    if (ctx->lib_name && strlen(ctx->lib_name) > 0) {
        if (!strstr(info->dlpi_name, ctx->lib_name)) return 0;
    }

    // Examine GOT table entries for matching target symbol
    for (int i = 0; i < info->dlpi_phnum; i++) {
        if (info->dlpi_phdr[i].p_type == PT_DYNAMIC) {
            ElfW(Dyn)* dyn = reinterpret_cast<ElfW(Dyn)*>(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);
            ElfW(Sym)* symtab = nullptr;
            const char* strtab = nullptr;
            ElfW(Addr)* got = nullptr;

            for (ElfW(Dyn)* d = dyn; d->d_tag != DT_NULL; d++) {
                if (d->d_tag == DT_SYMTAB) symtab = reinterpret_cast<ElfW(Sym)*>(info->dlpi_addr + d->d_un.d_ptr);
                else if (d->d_tag == DT_STRTAB) strtab = reinterpret_cast<const char*>(info->dlpi_addr + d->d_un.d_ptr);
                else if (d->d_tag == DT_PLTGOT) got = reinterpret_cast<ElfW(Addr)*>(info->dlpi_addr + d->d_un.d_ptr);
            }

            if (!symtab || !strtab || !got) continue;

            // Simple PLT scan
            for (size_t idx = 0; idx < 1024; idx++) {
                ElfW(Addr)* entry = &got[idx];
                if (*entry == reinterpret_cast<ElfW(Addr)>(ctx->hook_func)) {
                    continue;
                }
                
                Dl_info dlinfo;
                if (dladdr(reinterpret_cast<void*>(*entry), &dlinfo) && dlinfo.dli_sname) {
                    if (strcmp(dlinfo.dli_sname, ctx->symbol_name) == 0) {
                        ctx->orig_func = reinterpret_cast<void*>(*entry);
                        
                        uintptr_t page_start = reinterpret_cast<uintptr_t>(entry) & ~((uintptr_t)sysconf(_SC_PAGESIZE) - 1);
                        mprotect(reinterpret_cast<void*>(page_start), sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE);
                        *entry = reinterpret_cast<ElfW(Addr)>(ctx->hook_func);
                        mprotect(reinterpret_cast<void*>(page_start), sysconf(_SC_PAGESIZE), PROT_READ);
                        
                        LOGI("Successfully hooked symbol %s in %s", ctx->symbol_name, info->dlpi_name);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void* plt_hook_symbol(const char* lib_name, const char* symbol_name, void* hook_func) {
    HookContext ctx = { lib_name, symbol_name, hook_func, nullptr };
    dl_iterate_phdr(dl_iterate_callback, &ctx);
    return ctx.orig_func;
}

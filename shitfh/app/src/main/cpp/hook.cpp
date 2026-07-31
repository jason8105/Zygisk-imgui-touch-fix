#include "hook.h"
#include <dlfcn.h>
#include <link.h>
#include <elf.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <android/log.h>

#define LOG_TAG "ZygiskImGuiHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#if defined(__LP64__)
#define Elf_Sym Elf64_Sym
#define Elf_Dyn Elf64_Dyn
#define Elf_Rel Elf64_Rela
#define ELF_R_SYM ELF64_R_SYM
#else
#define Elf_Sym Elf32_Sym
#define Elf_Dyn Elf32_Dyn
#define Elf_Rel Elf32_Rel
#define ELF_R_SYM ELF32_R_SYM
#endif

struct HookContext {
    const char* symbol_name;
    void* target_func;
    void* new_func;
    void** old_func;
};

static int plt_hook_callback(struct dl_phdr_info* info, size_t size, void* data) {
    HookContext* ctx = static_cast<HookContext*>(data);
    if (!info->dlpi_name || !info->dlpi_name[0]) return 0;

    if (strstr(info->dlpi_name, "libzygisk.so") ||
        strstr(info->dlpi_name, "libc.so") ||
        strstr(info->dlpi_name, "libm.so") ||
        strstr(info->dlpi_name, "libdl.so") ||
        strstr(info->dlpi_name, "libart.so")) {
        return 0;
    }

    uintptr_t base = info->dlpi_addr;
    const ElfPhdr* phdr = info->dlpi_phdr;

    const Elf_Dyn* dynamic = nullptr;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dynamic = reinterpret_cast<const Elf_Dyn*>(base + phdr[i].p_vaddr);
            break;
        }
    }

    if (!dynamic) return 0;

    const Elf_Sym* symtab = nullptr;
    const char* strtab = nullptr;
    const Elf_Rel* rels = nullptr;
    size_t rel_count = 0;

    for (const Elf_Dyn* d = dynamic; d->d_tag != DT_NULL; d++) {
        if (d->d_tag == DT_SYMTAB) symtab = reinterpret_cast<const Elf_Sym*>(base + d->d_un.d_ptr);
        else if (d->d_tag == DT_STRTAB) strtab = reinterpret_cast<const char*>(base + d->d_un.d_ptr);
        else if (d->d_tag == DT_JMPREL) rels = reinterpret_cast<const Elf_Rel*>(base + d->d_un.d_ptr);
        else if (d->d_tag == DT_PLTRELSZ) rel_count = d->d_un.d_val / sizeof(Elf_Rel);
    }

    if (!rels || !symtab || !strtab) return 0;

    for (size_t i = 0; i < rel_count; i++) {
        size_t sym_idx = ELF_R_SYM(rels[i].r_info);
        const char* name = strtab + symtab[sym_idx].st_name;
        if (name && strcmp(name, ctx->symbol_name) == 0) {
            void** slot = reinterpret_cast<void**>(base + rels[i].r_offset);
            if (*slot != ctx->new_func) {
                if (ctx->old_func && !*ctx->old_func) {
                    *ctx->old_func = *slot;
                }
                uintptr_t page_start = reinterpret_cast<uintptr_t>(slot) & ~(getpagesize() - 1);
                mprotect(reinterpret_cast<void*>(page_start), getpagesize(), PROT_READ | PROT_WRITE);
                *slot = ctx->new_func;
                mprotect(reinterpret_cast<void*>(page_start), getpagesize(), PROT_READ);
                LOGI("Hooked PLT symbol %s in library %s", ctx->symbol_name, info->dlpi_name);
            }
        }
    }
    return 0;
}

void hook_plt_all(const char* symbol_name, void* new_func, void** old_func) {
    HookContext ctx;
    ctx.symbol_name = symbol_name;
    ctx.new_func = new_func;
    ctx.old_func = old_func;

    void* handle = dlopen(nullptr, RTLD_NOW);
    if (handle) {
        ctx.target_func = dlsym(handle, symbol_name);
        dlclose(handle);
    }

    dl_iterate_phdr(plt_hook_callback, &ctx);
}

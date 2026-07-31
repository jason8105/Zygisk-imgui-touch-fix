#include "hook_utils.h"
#include <dlfcn.h>
#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "ZygiskHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct plt_hook_data {
    const char *symbol_name;
    void *hook_func;
    void **orig_func;
    bool found;
};

static int dl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data) {
    auto *ctx = static_cast<plt_hook_data *>(data);
    if (!info->dlpi_name || !info->dlpi_addr) return 0;
    if (strstr(info->dlpi_name, "libzygisk.so")) return 0;

    ElfW(Addr) base = info->dlpi_addr;
    const ElfW(Phdr) *phdr = info->dlpi_phdr;
    const ElfW(Dyn) *dynamic = nullptr;

    for (int i = 0; i < info->dlpi_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dynamic = reinterpret_cast<const ElfW(Dyn) *>(base + phdr[i].p_vaddr);
            break;
        }
    }
    if (!dynamic) return 0;

    const ElfW(Sym) *symtab = nullptr;
    const char *strtab = nullptr;
    const ElfW(Rel) *rel = nullptr;
    const ElfW(Rela) *rela = nullptr;
    size_t rel_sz = 0, rela_sz = 0;

    for (const ElfW(Dyn) *d = dynamic; d->d_tag != DT_NULL; ++d) {
        switch (d->d_tag) {
            case DT_SYMTAB: symtab = reinterpret_cast<const ElfW(Sym) *>(base + d->d_un.d_ptr); break;
            case DT_STRTAB: strtab = reinterpret_cast<const char *>(base + d->d_un.d_ptr); break;
            case DT_JMPREL:
                rel = reinterpret_cast<const ElfW(Rel) *>(base + d->d_un.d_ptr);
                rela = reinterpret_cast<const ElfW(Rela) *>(base + d->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                rel_sz = d->d_un.d_val;
                rela_sz = d->d_un.d_val;
                break;
            default: break;
        }
    }

    if (!symtab || !strtab) return 0;

    if (rel && rel_sz) {
        size_t count = rel_sz / sizeof(ElfW(Rel));
        for (size_t i = 0; i < count; i++) {
            size_t sym_idx = ELF_R_SYM(rel[i].r_info);
            const char *name = strtab + symtab[sym_idx].st_name;
            if (name && strcmp(name, ctx->symbol_name) == 0) {
                void **got_entry = reinterpret_cast<void **>(base + rel[i].r_offset);
                if (*got_entry != ctx->hook_func) {
                    if (ctx->orig_func && !*ctx->orig_func) {
                        *ctx->orig_func = *got_entry;
                    }
                    uintptr_t page_start = reinterpret_cast<uintptr_t>(got_entry) & ~((uintptr_t)PAGE_SIZE - 1);
                    mprotect(reinterpret_cast<void *>(page_start), PAGE_SIZE, PROT_READ | PROT_WRITE);
                    *got_entry = ctx->hook_func;
                    mprotect(reinterpret_cast<void *>(page_start), PAGE_SIZE, PROT_READ);
                    ctx->found = true;
                    LOGI("Hooked PLT symbol %s in %s", ctx->symbol_name, info->dlpi_name);
                }
            }
        }
    }

    if (rela && rela_sz) {
        size_t count = rela_sz / sizeof(ElfW(Rela));
        for (size_t i = 0; i < count; i++) {
            size_t sym_idx = ELF_R_SYM(rela[i].r_info);
            const char *name = strtab + symtab[sym_idx].st_name;
            if (name && strcmp(name, ctx->symbol_name) == 0) {
                void **got_entry = reinterpret_cast<void **>(base + rela[i].r_offset);
                if (*got_entry != ctx->hook_func) {
                    if (ctx->orig_func && !*ctx->orig_func) {
                        *ctx->orig_func = *got_entry;
                    }
                    uintptr_t page_start = reinterpret_cast<uintptr_t>(got_entry) & ~((uintptr_t)PAGE_SIZE - 1);
                    mprotect(reinterpret_cast<void *>(page_start), PAGE_SIZE, PROT_READ | PROT_WRITE);
                    *got_entry = ctx->hook_func;
                    mprotect(reinterpret_cast<void *>(page_start), PAGE_SIZE, PROT_READ);
                    ctx->found = true;
                    LOGI("Hooked PLTA symbol %s in %s", ctx->symbol_name, info->dlpi_name);
                }
            }
        }
    }

    return 0;
}

bool hook_plt(const char *module_pattern, const char *symbol_name, void *hook_func, void **orig_func) {
    plt_hook_data data = { symbol_name, hook_func, orig_func, false };
    dl_iterate_phdr(dl_iterate_callback, &data);
    return data.found;
}

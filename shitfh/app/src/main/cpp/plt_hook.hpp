#ifndef PLT_HOOK_HPP
#ifndef PLT_HOOK_HPP_
#define PLT_HOOK_HPP_

#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>

namespace PltHook {

struct HookContext {
    const char* target_lib;
    const char* target_symbol;
    void* new_fn;
    void** old_fn;
    bool success;
};

inline bool hook_symbol_in_library(const char* lib_name, const char* symbol_name, void* new_func, void** old_func) {
    HookContext ctx = { lib_name, symbol_name, new_func, old_func, false };

    dl_iterate_phdr([](struct dl_phdr_info *info, size_t size, void *data) -> int {
        auto* c = reinterpret_cast<HookContext*>(data);
        if (!info->dlpi_name) return 0;

        if (c->target_lib && !strstr(info->dlpi_name, c->target_lib)) {
            return 0;
        }

        uintptr_t base = info->dlpi_addr;
        const ElfW(Phdr) *phdr = info->dlpi_phdr;

        const ElfW(Dyn) *dynamic = nullptr;
        for (int i = 0; i < info->dlpi_phnum; i++) {
            if (phdr[i].p_type == PT_DYNAMIC) {
                dynamic = reinterpret_cast<const ElfW(Dyn)*>(base + phdr[i].p_vaddr);
                break;
            }
        }
        if (!dynamic) return 0;

        const ElfW(Sym) *symtab = nullptr;
        const char *strtab = nullptr;
        const ElfW(Rela) *rela = nullptr;
        size_t rela_sz = 0;
        const ElfW(Rel) *rel = nullptr;
        size_t rel_sz = 0;

        for (const ElfW(Dyn) *d = dynamic; d->d_tag != DT_NULL; d++) {
            switch (d->d_tag) {
                case DT_SYMTAB: symtab = reinterpret_cast<const ElfW(Sym)*>(base + d->d_un.d_ptr); break;
                case DT_STRTAB: strtab = reinterpret_cast<const char*>(base + d->d_un.d_ptr); break;
                case DT_JMPREL:
#if defined(__aarch64__) || defined(__x86_64__)
                    rela = reinterpret_cast<const ElfW(Rela)*>(base + d->d_un.d_ptr);
#else
                    rel = reinterpret_cast<const ElfW(Rel)*>(base + d->d_un.d_ptr);
#endif
                    break;
                case DT_PLTRELSZ:
#if defined(__aarch64__) || defined(__x86_64__)
                    rela_sz = d->d_un.d_val / sizeof(ElfW(Rela));
#else
                    rel_sz = d->d_un.d_val / sizeof(ElfW(Rel));
#endif
                    break;
            }
        }

        if (!symtab || !strtab) return 0;

#if defined(__aarch64__) || defined(__x86_64__)
        if (rela) {
            for (size_t i = 0; i < rela_sz; i++) {
                unsigned long sym_idx = ELF64_R_SYM(rela[i].r_info);
                if (sym_idx && strcmp(strtab + symtab[sym_idx].st_name, c->target_symbol) == 0) {
                    void **target_addr = reinterpret_cast<void**>(base + rela[i].r_offset);
                    if (*target_addr != c->new_fn) {
                        if (c->old_fn && !*c->old_fn) {
                            *c->old_fn = *target_addr;
                        }
                        uintptr_t page_start = reinterpret_cast<uintptr_t>(target_addr) & ~0xFFF;
                        mprotect(reinterpret_cast<void*>(page_start), 0x1000, PROT_READ | PROT_WRITE);
                        *target_addr = c->new_fn;
                        mprotect(reinterpret_cast<void*>(page_start), 0x1000, PROT_READ);
                        c->success = true;
                    }
                }
            }
        }
#else
        if (rel) {
            for (size_t i = 0; i < rel_sz; i++) {
                unsigned long sym_idx = ELF32_R_SYM(rel[i].r_info);
                if (sym_idx && strcmp(strtab + symtab[sym_idx].st_name, c->target_symbol) == 0) {
                    void **target_addr = reinterpret_cast<void**>(base + rel[i].r_offset);
                    if (*target_addr != c->new_fn) {
                        if (c->old_fn && !*c->old_fn) {
                            *c->old_fn = *target_addr;
                        }
                        uintptr_t page_start = reinterpret_cast<uintptr_t>(target_addr) & ~0xFFF;
                        mprotect(reinterpret_cast<void*>(page_start), 0x1000, PROT_READ | PROT_WRITE);
                        *target_addr = c->new_fn;
                        mprotect(reinterpret_cast<void*>(page_start), 0x1000, PROT_READ);
                        c->success = true;
                    }
                }
            }
        }
#endif
        return 0;
    }, &ctx);

    return ctx.success;
}

inline bool hook_symbol_all(const char* symbol_name, void* new_func, void** old_func) {
    return hook_symbol_in_library(nullptr, symbol_name, new_func, old_func);
}

} // namespace PltHook

#endif
#endif

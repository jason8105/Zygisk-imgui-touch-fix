#pragma once

#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace plt_hook {

#if defined(__LP64__)
using Elf_Ehdr = Elf64_Ehdr;
using Elf_Phdr = Elf64_Phdr;
using Elf_Dyn = Elf64_Dyn;
using Elf_Sym = Elf64_Sym;
using Elf_Rela = Elf64_Rela;
using Elf_Rel = Elf64_Rel;
#define ELF_R_SYM ELF64_R_SYM
#else
using Elf_Ehdr = Elf32_Ehdr;
using Elf_Phdr = Elf32_Phdr;
using Elf_Dyn = Elf32_Dyn;
using Elf_Sym = Elf32_Sym;
using Elf_Rel = Elf32_Rel;
#define ELF_R_SYM ELF32_R_SYM
#endif

inline bool hook_plt_in_header(dl_phdr_info* info, const char* symbol_name, void* new_func, void** old_func) {
    if (!info || !info->dlpi_name) return false;

    const Elf_Phdr* phdr = info->dlpi_phdr;
    ElfAddr load_bias = info->dlpi_addr;

    const Elf_Dyn* dynamic = nullptr;
    for (int i = 0; i < info->dlpi_phnum; ++i) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dynamic = reinterpret_cast<const Elf_Dyn*>(load_bias + phdr[i].p_vaddr);
            break;
        }
    }
    if (!dynamic) return false;

    const Elf_Sym* symtab = nullptr;
    const char* strtab = nullptr;
    const Elf_Rela* rela = nullptr;
    size_t relasz = 0;
    const Elf_Rel* rel = nullptr;
    size_t relsz = 0;
    size_t pltrelsz = 0;
    const void* jmprel = nullptr;

    for (const Elf_Dyn* d = dynamic; d->d_tag != DT_NULL; ++d) {
        switch (d->d_tag) {
            case DT_SYMTAB: symtab = reinterpret_cast<const Elf_Sym*>(load_bias + d->d_un.d_ptr); break;
            case DT_STRTAB: strtab = reinterpret_cast<const char*>(load_bias + d->d_un.d_ptr); break;
            case DT_RELA: rela = reinterpret_cast<const Elf_Rela*>(load_bias + d->d_un.d_ptr); break;
            case DT_RELASZ: relasz = d->d_un.d_val; break;
            case DT_REL: rel = reinterpret_cast<const Elf_Rel*>(load_bias + d->d_un.d_ptr); break;
            case DT_RELSZ: relsz = d->d_un.d_val; break;
            case DT_JMPREL: jmprel = reinterpret_cast<const void*>(load_bias + d->d_un.d_ptr); break;
            case DT_PLTRELSZ: pltrelsz = d->d_un.d_val; break;
        }
    }

    if (!symtab || !strtab) return false;

    auto check_and_replace = [&](ElfAddr rel_addr, uint32_t r_info_sym) -> bool {
        uint32_t sym_idx = r_info_sym;
        const char* name = strtab + symtab[sym_idx].st_name;
        if (name && strcmp(name, symbol_name) == 0) {
            void** target = reinterpret_cast<void**>(load_bias + rel_addr);
            if (old_func && *old_func == nullptr) {
                *old_func = *target;
            }
            if (*target != new_func) {
                uintptr_t page_start = reinterpret_cast<uintptr_t>(target) & ~(PAGE_SIZE - 1);
                mprotect(reinterpret_cast<void*>(page_start), PAGE_SIZE, PROT_READ | PROT_WRITE);
                *target = new_func;
                mprotect(reinterpret_cast<void*>(page_start), PAGE_SIZE, PROT_READ);
                return true;
            }
        }
        return false;
    };

    bool hooked = false;

#if defined(__LP64__)
    if (rela && relasz) {
        size_t count = relasz / sizeof(Elf_Rela);
        for (size_t i = 0; i < count; ++i) {
            if (check_and_replace(rela[i].r_offset, ELF64_R_SYM(rela[i].r_info))) hooked = true;
        }
    }
#endif

    if (rel && relsz) {
        size_t count = relsz / sizeof(Elf_Rel);
        for (size_t i = 0; i < count; ++i) {
            if (check_and_replace(rel[i].r_offset, ELF_R_SYM(rel[i].r_info))) hooked = true;
        }
    }

    if (jmprel && pltrelsz) {
#if defined(__LP64__)
        const Elf_Rela* plt_rela = reinterpret_cast<const Elf_Rela*>(jmprel);
        size_t count = pltrelsz / sizeof(Elf_Rela);
        for (size_t i = 0; i < count; ++i) {
            if (check_and_replace(plt_rela[i].r_offset, ELF64_R_SYM(plt_rela[i].r_info))) hooked = true;
        }
#else
        const Elf_Rel* plt_rel = reinterpret_cast<const Elf_Rel*>(jmprel);
        size_t count = pltrelsz / sizeof(Elf_Rel);
        for (size_t i = 0; i < count; ++i) {
            if (check_and_replace(plt_rel[i].r_offset, ELF32_R_SYM(plt_rel[i].r_info))) hooked = true;
        }
#endif
    }

    return hooked;
}

inline void hook_all_modules(const char* symbol_name, void* new_func, void** old_func) {
    struct Context {
        const char* symbol;
        void* new_f;
        void** old_f;
    } ctx = { symbol_name, new_func, old_func };

    dl_iterate_phdr([](dl_phdr_info* info, size_t, void* data) -> int {
        auto* c = reinterpret_cast<Context*>(data);
        hook_plt_in_header(info, c->symbol, c->new_f, c->old_f);
        return 0;
    }, &ctx);
}

} // namespace plt_hook

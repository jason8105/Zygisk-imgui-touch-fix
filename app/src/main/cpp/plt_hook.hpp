#pragma once

#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace PLTHook {

struct HookTarget {
    const char* symbol_name;
    void* new_func;
    void** old_func;
};

inline bool hook_symbol_in_image(dl_phdr_info* info, const char* symbol_name, void* new_func, void** old_func) {
    if (!info || !info->dlpi_name) return false;

    const ElfW(Phdr)* phdr = info->dlpi_phdr;
    const ElfW(Dyn)* dyn = nullptr;
    ElfW(Addr) load_bias = info->dlpi_addr;

    for (int i = 0; i < info->dlpi_phnum; ++i) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = reinterpret_cast<const ElfW(Dyn)*>(load_bias + phdr[i].p_vaddr);
            break;
        }
    }

    if (!dyn) return false;

    const ElfW(Sym)* symtab = nullptr;
    const char* strtab = nullptr;
    const ElfW(Rel)* rel = nullptr;
    const ElfW(Rela)* rela = nullptr;
    size_t rel_sz = 0, rela_sz = 0;
    size_t plt_rel_sz = 0;
    uint32_t plt_rel_type = DT_NULL;
    const void* plt_rel = nullptr;

    for (const ElfW(Dyn)* d = dyn; d->d_tag != DT_NULL; ++d) {
        switch (d->d_tag) {
            case DT_SYMTAB:
                symtab = reinterpret_cast<const ElfW(Sym)*>(load_bias + d->d_un.d_ptr);
                break;
            case DT_STRTAB:
                strtab = reinterpret_cast<const char*>(load_bias + d->d_un.d_ptr);
                break;
            case DT_REL:
                rel = reinterpret_cast<const ElfW(Rel)*>(load_bias + d->d_un.d_ptr);
                break;
            case DT_RELSZ:
                rel_sz = d->d_un.d_val;
                break;
            case DT_RELA:
                rela = reinterpret_cast<const ElfW(Rela)*>(load_bias + d->d_un.d_ptr);
                break;
            case DT_RELASZ:
                rela_sz = d->d_un.d_val;
                break;
            case DT_PLTREL:
                plt_rel_type = d->d_un.d_val;
                break;
            case DT_JMPREL:
                plt_rel = reinterpret_cast<const void*>(load_bias + d->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                plt_rel_sz = d->d_un.d_val;
                break;
        }
    }

    if (!symtab || !strtab) return false;

    bool hooked = false;

    auto check_and_patch = [&](ElfW(Addr) r_offset, ElfW(Word) r_info) {
        uint32_t sym_idx = ELF64_R_SYM(r_info);
#if defined(__arm__)
        sym_idx = ELF32_R_SYM(r_info);
#endif
        const char* name = strtab + symtab[sym_idx].st_name;
        if (name && strcmp(name, symbol_name) == 0) {
            void** target_addr = reinterpret_cast<void**>(load_bias + r_offset);
            if (*target_addr != new_func) {
                if (old_func && !*old_func) {
                    *old_func = *target_addr;
                }

                uintptr_t page_start = reinterpret_cast<uintptr_t>(target_addr) & ~(PAGE_SIZE - 1);
                mprotect(reinterpret_cast<void*>(page_start), PAGE_SIZE, PROT_READ | PROT_WRITE);
                *target_addr = new_func;
                mprotect(reinterpret_cast<void*>(page_start), PAGE_SIZE, PROT_READ);
                hooked = true;
            }
        }
    };

    if (rel && rel_sz > 0) {
        size_t count = rel_sz / sizeof(ElfW(Rel));
        for (size_t i = 0; i < count; ++i) {
            check_and_patch(rel[i].r_offset, rel[i].r_info);
        }
    }

    if (rela && rela_sz > 0) {
        size_t count = rela_sz / sizeof(ElfW(Rela));
        for (size_t i = 0; i < count; ++i) {
            check_and_patch(rela[i].r_offset, rela[i].r_info);
        }
    }

    if (plt_rel && plt_rel_sz > 0) {
        if (plt_rel_type == DT_REL) {
            const ElfW(Rel)* r = reinterpret_cast<const ElfW(Rel)*>(plt_rel);
            size_t count = plt_rel_sz / sizeof(ElfW(Rel));
            for (size_t i = 0; i < count; ++i) {
                check_and_patch(r[i].r_offset, r[i].r_info);
            }
        } else if (plt_rel_type == DT_RELA) {
            const ElfW(Rela)* r = reinterpret_cast<const ElfW(Rela)*>(plt_rel);
            size_t count = plt_rel_sz / sizeof(ElfW(Rela));
            for (size_t i = 0; i < count; ++i) {
                check_and_patch(r[i].r_offset, r[i].r_info);
            }
        }
    }

    return hooked;
}

inline void hook_all(const std::vector<HookTarget>& targets) {
    dl_iterate_phdr([](struct dl_phdr_info *info, size_t size, void *data) -> int {
        auto* targets = reinterpret_cast<const std::vector<HookTarget>*>(data);
        for (const auto& target : *targets) {
            hook_symbol_in_image(info, target.symbol_name, target.new_func, target.old_func);
        }
        return 0;
    }, const_cast<void*>(reinterpret_cast<const void*>(&targets)));
}

} // namespace PLTHook

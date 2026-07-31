#include "plt_hook.h"
#include <dlfcn.h>
#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <android/log.h>

#define LOG_TAG "PLTHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#if defined(__LP64__)
#define ELF_R_SYM ELF64_R_SYM
#else
#define ELF_R_SYM ELF32_R_SYM
#endif

namespace PLTHook {

struct HookTarget {
    const char* symbol_name;
    void* new_func;
    void** old_func;
};

static std::vector<HookTarget> g_hooks;

void RegisterHook(const char* symbol_name, void* new_func, void** old_func) {
    g_hooks.push_back({symbol_name, new_func, old_func});
}

static int PhdrCallback(struct dl_phdr_info *info, size_t size, void *data) {
    if (!info || !info->dlpi_name || info->dlpi_addr == 0) return 0;

    ElfW(Addr) base = info->dlpi_addr;
    const ElfW(Phdr) *phdr = info->dlpi_phdr;
    size_t phnum = info->dlpi_phnum;

    const ElfW(Dyn) *dyn = nullptr;
    for (size_t i = 0; i < phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = reinterpret_cast<const ElfW(Dyn) *>(base + phdr[i].p_vaddr);
            break;
        }
    }

    if (!dyn) return 0;

    const ElfW(Sym) *symtab = nullptr;
    const char *strtab = nullptr;
    const ElfW(Rel) *rel = nullptr;
    const ElfW(Rela) *rela = nullptr;
    size_t relsz = 0, relasz = 0, pltrelsz = 0;
    size_t pltrel = 0;
    const void *jmprel = nullptr;

    for (const ElfW(Dyn) *d = dyn; d->d_tag != DT_NULL; d++) {
        switch (d->d_tag) {
            case DT_SYMTAB: symtab = reinterpret_cast<const ElfW(Sym) *>(base + d->d_un.d_ptr); break;
            case DT_STRTAB: strtab = reinterpret_cast<const char *>(base + d->d_un.d_ptr); break;
            case DT_REL: rel = reinterpret_cast<const ElfW(Rel) *>(base + d->d_un.d_ptr); break;
            case DT_RELSZ: relsz = d->d_un.d_val; break;
            case DT_RELA: rela = reinterpret_cast<const ElfW(Rela) *>(base + d->d_un.d_ptr); break;
            case DT_RELASZ: relasz = d->d_un.d_val; break;
            case DT_JMPREL: jmprel = reinterpret_cast<const void *>(base + d->d_un.d_ptr); break;
            case DT_PLTRELSZ: pltrelsz = d->d_un.d_val; break;
            case DT_PLTREL: pltrel = d->d_un.d_val; break;
        }
    }

    if (!symtab || !strtab) return 0;

    auto ScanRelocations = [&](const auto *rel_ptr, size_t count) {
        if (!rel_ptr) return;
        for (size_t i = 0; i < count; i++) {
            size_t sym_idx = ELF_R_SYM(rel_ptr[i].r_info);
            const char *sym_name = strtab + symtab[sym_idx].st_name;

            for (auto &hook : g_hooks) {
                if (sym_name && strcmp(sym_name, hook.symbol_name) == 0 && hook.new_func != nullptr) {
                    void **got_entry = reinterpret_cast<void **>(base + rel_ptr[i].r_offset);
                    if (*got_entry != hook.new_func) {
                        if (hook.old_func && *hook.old_func == nullptr) {
                            *hook.old_func = *got_entry;
                        }
                        long page_size = sysconf(_SC_PAGESIZE);
                        uintptr_t page_start = reinterpret_cast<uintptr_t>(got_entry) & ~(page_size - 1);
                        mprotect(reinterpret_cast<void *>(page_start), page_size, PROT_READ | PROT_WRITE);
                        *got_entry = hook.new_func;
                        mprotect(reinterpret_cast<void *>(page_start), page_size, PROT_READ);
                        LOGI("Hooked PLT symbol %s in %s", hook.symbol_name, info->dlpi_name);
                    }
                }
            }
        }
    };

    if (jmprel && pltrelsz > 0) {
        if (pltrel == DT_RELA) {
            ScanRelocations(reinterpret_cast<const ElfW(Rela) *>(jmprel), pltrelsz / sizeof(ElfW(Rela)));
        } else {
            ScanRelocations(reinterpret_cast<const ElfW(Rel) *>(jmprel), pltrelsz / sizeof(ElfW(Rel)));
        }
    }

    if (rel && relsz > 0) {
        ScanRelocations(rel, relsz / sizeof(ElfW(Rel)));
    }
    if (rela && relasz > 0) {
        ScanRelocations(rela, relasz / sizeof(ElfW(Rela)));
    }

    return 0;
}

void ApplyHooks() {
    dl_iterate_phdr(PhdrCallback, nullptr);
}

} // namespace PLTHook

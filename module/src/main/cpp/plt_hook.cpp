#include "plt_hook.hpp"
#include <link.h>
#include <elf.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "PLTHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct HookContext {
    const char* symbol_name;
    void* new_func;
    void** old_func;
};

#if defined(__LP64__)
typedef Elf64_Rela Elf_Rel_t;
typedef Elf64_Sym Elf_Sym_t;
typedef Elf64_Ehdr Elf_Ehdr_t;
typedef Elf64_Phdr Elf_Phdr_t;
typedef Elf64_Dyn Elf_Dyn_t;
#define ELF_R_SYM(info) ELF64_R_SYM(info)
#else
typedef Elf32_Rel Elf_Rel_t;
typedef Elf32_Sym Elf_Sym_t;
typedef Elf32_Ehdr Elf_Ehdr_t;
typedef Elf32_Phdr Elf_Phdr_t;
typedef Elf32_Dyn Elf_Dyn_t;
#define ELF_R_SYM(info) ELF32_R_SYM(info)
#endif

static int phdr_callback(struct dl_phdr_info *info, size_t size, void *data) {
    HookContext* ctx = reinterpret_cast<HookContext*>(data);
    if (!info || !info->dlpi_name || !ctx) return 0;

    uintptr_t base = info->dlpi_addr;
    if (base == 0) return 0;

    const Elf_Phdr_t* phdr = info->dlpi_phdr;
    const Elf_Dyn_t* dynamic = nullptr;

    for (int i = 0; i < info->dlpi_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dynamic = reinterpret_cast<const Elf_Dyn_t*>(base + phdr[i].p_vaddr);
            break;
        }
    }

    if (!dynamic) return 0;

    const char* strtab = nullptr;
    const Elf_Sym_t* symtab = nullptr;
    const Elf_Rel_t* jmprel = nullptr;
    size_t pltrelsz = 0;

    for (const Elf_Dyn_t* d = dynamic; d->d_tag != DT_NULL; ++d) {
        switch (d->d_tag) {
            case DT_STRTAB:
                strtab = reinterpret_cast<const char*>(base + d->d_un.d_ptr);
                break;
            case DT_SYMTAB:
                symtab = reinterpret_cast<const Elf_Sym_t*>(base + d->d_un.d_ptr);
                break;
            case DT_JMPREL:
                jmprel = reinterpret_cast<const Elf_Rel_t*>(base + d->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                pltrelsz = d->d_un.d_val;
                break;
        }
    }

    if (!strtab || !symtab || !jmprel || pltrelsz == 0) return 0;

    size_t rel_count = pltrelsz / sizeof(Elf_Rel_t);
    size_t page_size = sysconf(_SC_PAGESIZE);

    for (size_t i = 0; i < rel_count; i++) {
        const Elf_Rel_t* rel = &jmprel[i];
        size_t sym_idx = ELF_R_SYM(rel->r_info);
        const char* name = strtab + symtab[sym_idx].st_name;

        if (name && strcmp(name, ctx->symbol_name) == 0) {
            void** got_entry = reinterpret_cast<void**>(base + rel->r_offset);
            if (*got_entry == ctx->new_func) continue;

            if (ctx->old_func && *ctx->old_func == nullptr) {
                *ctx->old_func = *got_entry;
            }

            uintptr_t page_start = reinterpret_cast<uintptr_t>(got_entry) & ~(page_size - 1);
            mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_WRITE);
            *got_entry = ctx->new_func;
            mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ);

            LOGI("Successfully hooked PLT symbol %s at %p in %s", ctx->symbol_name, got_entry, info->dlpi_name);
        }
    }

    return 0;
}

namespace plt_hook {

void hook_all_modules(const char* symbol_name, void* new_func, void** old_func) {
    HookContext ctx{symbol_name, new_func, old_func};
    dl_iterate_phdr(phdr_callback, &ctx);
}

} // namespace plt_hook

#include "plt_hook.h"
#include <link.h>
#include <elf.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <android/log.h>

#define LOG_TAG "ZygiskPLTHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#if defined(__LP64__)
#define Elf_Phdr Elf64_Phdr
#define Elf_Dyn Elf64_Dyn
#define Elf_Sym Elf64_Sym
#define Elf_Rela Elf64_Rela
#define Elf_Rel Elf64_Rel
#define ELF_R_SYM ELF64_R_SYM
#else
#define Elf_Phdr Elf32_Phdr
#define Elf_Dyn Elf32_Dyn
#define Elf_Sym Elf32_Sym
#define Elf_Rela Elf32_Rela
#define Elf_Rel Elf32_Rel
#define ELF_R_SYM ELF32_R_SYM
#endif

struct HookContext {
    const char* target_lib;
    const char* symbol_name;
    void* new_func;
    void** old_func;
    bool found;
};

static bool replace_got_entry(uintptr_t got_addr, void* new_func, void** old_func) {
    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = got_addr & ~(page_size - 1);

    void* current_val = *reinterpret_cast<void**>(got_addr);
    if (current_val == new_func) return true;

    if (old_func && *old_func == nullptr) {
        *old_func = current_val;
    }

    if (mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_WRITE) != 0) {
        LOGE("mprotect failed at %p", reinterpret_cast<void*>(page_start));
        return false;
    }

    *reinterpret_cast<void**>(got_addr) = new_func;

    mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ);
    return true;
}

static int phdr_callback(struct dl_phdr_info *info, size_t size, void *data) {
    auto* ctx = reinterpret_cast<HookContext*>(data);

    if (ctx->target_lib && info->dlpi_name && strstr(info->dlpi_name, ctx->target_lib) == nullptr) {
        return 0;
    }

    if (info->dlpi_name && strstr(info->dlpi_name, "libzygisk.so") != nullptr) {
        return 0;
    }

    Elf_Addr base = info->dlpi_addr;
    const Elf_Phdr *phdr = info->dlpi_phdr;

    const Elf_Dyn *dynamic = nullptr;
    for (int i = 0; i < info->dlpi_phnum; ++i) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dynamic = reinterpret_cast<const Elf_Dyn*>(base + phdr[i].p_vaddr);
            break;
        }
    }

    if (!dynamic) return 0;

    const Elf_Sym *symtab = nullptr;
    const char *strtab = nullptr;
    const void *jmprel = nullptr;
    size_t jmprelsz = 0;
    bool is_rela = false;

    for (const Elf_Dyn *d = dynamic; d->d_tag != DT_NULL; ++d) {
        switch (d->d_tag) {
            case DT_SYMTAB:
                symtab = reinterpret_cast<const Elf_Sym*>(base + d->d_un.d_ptr);
                break;
            case DT_STRTAB:
                strtab = reinterpret_cast<const char*>(base + d->d_un.d_ptr);
                break;
            case DT_JMPREL:
                jmprel = reinterpret_cast<const void*>(base + d->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                jmprelsz = d->d_un.d_val;
                break;
            case DT_PLTREL:
                if (d->d_un.d_val == DT_RELA) is_rela = true;
                break;
        }
    }

    if (!symtab || !strtab || !jmprel || jmprelsz == 0) return 0;

    size_t entry_size = is_rela ? sizeof(Elf_Rela) : sizeof(Elf_Rel);
    size_t count = jmprelsz / entry_size;

    for (size_t i = 0; i < count; ++i) {
        uintptr_t offset = 0;
        uint32_t sym_idx = 0;

        if (is_rela) {
            const auto* rela = reinterpret_cast<const Elf_Rela*>(reinterpret_cast<uintptr_t>(jmprel) + i * entry_size);
            offset = rela->r_offset;
            sym_idx = ELF_R_SYM(rela->r_info);
        } else {
            const auto* rel = reinterpret_cast<const Elf_Rel*>(reinterpret_cast<uintptr_t>(jmprel) + i * entry_size);
            offset = rel->r_offset;
            sym_idx = ELF_R_SYM(rel->r_info);
        }

        if (sym_idx == 0) continue;

        const char* name = strtab + symtab[sym_idx].st_name;
        if (name && strcmp(name, ctx->symbol_name) == 0) {
            uintptr_t got_addr = base + offset;
            if (replace_got_entry(got_addr, ctx->new_func, ctx->old_func)) {
                ctx->found = true;
                LOGI("Hooked GOT entry %s at %p", ctx->symbol_name, (void*)got_addr);
            }
        }
    }

    return 0;
}

namespace PltHook {
    bool HookSymbol(const char* lib_name, const char* symbol_name, void* new_func, void** old_func) {
        HookContext ctx = { lib_name, symbol_name, new_func, old_func, false };
        dl_iterate_phdr(phdr_callback, &ctx);
        return ctx.found;
    }

    bool HookAll(const char* symbol_name, void* new_func, void** old_func) {
        HookContext ctx = { nullptr, symbol_name, new_func, old_func, false };
        dl_iterate_phdr(phdr_callback, &ctx);
        return ctx.found;
    }
}

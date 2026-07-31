#include "hook.h"
#include <link.h>
#include <elf.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string.h>
#include <android/log.h>
#include <unistd.h>

#define LOG_TAG "ZygiskHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#if defined(__LP64__)
#define Elf_Phdr Elf64_Phdr
#define Elf_Sym Elf64_Sym
#define Elf_Dyn Elf64_Dyn
#define Elf_Rel Elf64_Rela
#define ELF_R_SYM ELF64_R_SYM
#else
#define Elf_Phdr Elf32_Phdr
#define Elf_Sym Elf32_Sym
#define Elf_Dyn Elf32_Dyn
#define Elf_Rel Elf32_Rel
#define ELF_R_SYM ELF32_R_SYM
#endif

struct HookContext {
    const char* target_lib;
    const char* symbol_name;
    void* hook_func;
    void** orig_func;
    bool success;
};

static int dl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data) {
    HookContext* ctx = static_cast<HookContext*>(data);
    if (!info->dlpi_name) return 0;

    if (ctx->target_lib && strlen(ctx->target_lib) > 0 && !strstr(info->dlpi_name, ctx->target_lib)) {
        return 0;
    }

    uintptr_t load_addr = info->dlpi_addr;
    const Elf_Phdr* phdr = info->dlpi_phdr;

    const Elf_Dyn* dynamic = nullptr;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dynamic = reinterpret_cast<const Elf_Dyn*>(load_addr + phdr[i].p_vaddr);
            break;
        }
    }

    if (!dynamic) return 0;

    const Elf_Sym* symtab = nullptr;
    const char* strtab = nullptr;
    const Elf_Rel* rel_plt = nullptr;
    size_t rel_plt_sz = 0;

    for (const Elf_Dyn* d = dynamic; d->d_tag != DT_NULL; ++d) {
        if (d->d_tag == DT_SYMTAB) symtab = reinterpret_cast<const Elf_Sym*>(load_addr + d->d_un.d_ptr);
        else if (d->d_tag == DT_STRTAB) strtab = reinterpret_cast<const char*>(load_addr + d->d_un.d_ptr);
        else if (d->d_tag == DT_JMPREL) rel_plt = reinterpret_cast<const Elf_Rel*>(load_addr + d->d_un.d_ptr);
        else if (d->d_tag == DT_PLTRELSZ) rel_plt_sz = d->d_un.d_val;
    }

    if (!symtab || !strtab || !rel_plt) return 0;

    size_t count = rel_plt_sz / sizeof(Elf_Rel);
    for (size_t i = 0; i < count; ++i) {
        size_t sym_idx = ELF_R_SYM(rel_plt[i].r_info);
        const char* sym_name = strtab + symtab[sym_idx].st_name;

        if (strcmp(sym_name, ctx->symbol_name) == 0) {
            void** got_entry = reinterpret_cast<void**>(load_addr + rel_plt[i].r_offset);
            if (*got_entry != ctx->hook_func) {
                if (ctx->orig_func && !*ctx->orig_func) {
                    *ctx->orig_func = *got_entry;
                }
                long page_size = sysconf(_SC_PAGESIZE);
                uintptr_t page_start = reinterpret_cast<uintptr_t>(got_entry) & ~(page_size - 1);
                mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_WRITE);
                *got_entry = ctx->hook_func;
                mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ);
                ctx->success = true;
                LOGI("Hooked %s GOT entry at %p in %s", ctx->symbol_name, got_entry, info->dlpi_name);
            }
        }
    }

    return 0;
}

bool plt_hook_symbol(const char* target_library, const char* symbol_name, void* hook_func, void** orig_func) {
    HookContext ctx = { target_library, symbol_name, hook_func, orig_func, false };
    dl_iterate_phdr(dl_iterate_callback, &ctx);
    return ctx.success;
}

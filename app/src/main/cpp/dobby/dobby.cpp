#include "dobby.h"
#include <link.h>
#include <elf.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "PLTHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct HookContext {
    const char* lib_name;
    const char* symbol_name;
    void* new_func;
    void** old_func;
    bool hooked;
};

static int plt_hook_callback(struct dl_phdr_info *info, size_t size, void *data) {
    HookContext* ctx = static_cast<HookContext*>(data);
    if (!info->dlpi_name) return 0;

    if (ctx->lib_name && strlen(ctx->lib_name) > 0) {
        if (!strstr(info->dlpi_name, ctx->lib_name)) {
            return 0;
        }
    }

    ElfW(Addr) base = info->dlpi_addr;
    const ElfW(Phdr) *phdr = info->dlpi_phdr;
    const ElfW(Dyn) *dyn = nullptr;

    for (int i = 0; i < info->dlpi_phnum; ++i) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = reinterpret_cast<const ElfW(Dyn)*>(base + phdr[i].p_vaddr);
            break;
        }
    }

    if (!dyn) return 0;

    const ElfW(Sym) *symtab = nullptr;
    const char *strtab = nullptr;
    const ElfW(Rel) *rel = nullptr;
    const ElfW(Rela) *rela = nullptr;
    size_t relsz = 0, relasz = 0;

    for (const ElfW(Dyn) *d = dyn; d->d_tag != DT_NULL; ++d) {
        switch (d->d_tag) {
            case DT_SYMTAB: symtab = reinterpret_cast<const ElfW(Sym)*>(base + d->d_un.d_ptr); break;
            case DT_STRTAB: strtab = reinterpret_cast<const char*>(base + d->d_un.d_ptr); break;
            case DT_JMPREL:
                rel = reinterpret_cast<const ElfW(Rel)*>(base + d->d_un.d_ptr);
                rela = reinterpret_cast<const ElfW(Rela)*>(base + d->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                relsz = d->d_un.d_val;
                relasz = d->d_un.d_val;
                break;
        }
    }

    if (!symtab || !strtab) return 0;

    static long page_size = sysconf(_SC_PAGESIZE);
    static uintptr_t page_mask = ~(page_size - 1);

    auto check_and_replace = [&](ElfW(Addr) r_offset, ElfW(Word) r_info) {
        size_t sym_idx = ELFW(R_SYM)(r_info);
        const char *sym_name = strtab + symtab[sym_idx].st_name;

        if (strcmp(sym_name, ctx->symbol_name) == 0) {
            void **got_entry = reinterpret_cast<void**>(base + r_offset);
            if (ctx->old_func && *got_entry != ctx->new_func) {
                *ctx->old_func = *got_entry;
            }
            if (ctx->new_func) {
                uintptr_t page_start = reinterpret_cast<uintptr_t>(got_entry) & page_mask;
                mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_WRITE);
                *got_entry = ctx->new_func;
                mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ);
                ctx->hooked = true;
            }
        }
    };

    if (rel) {
        size_t count = relsz / sizeof(ElfW(Rel));
        for (size_t i = 0; i < count; ++i) {
            check_and_replace(rel[i].r_offset, rel[i].r_info);
        }
    }

    if (rela) {
        size_t count = relasz / sizeof(ElfW(Rela));
        for (size_t i = 0; i < count; ++i) {
            check_and_replace(rela[i].r_offset, rela[i].r_info);
        }
    }

    return 0;
}

extern "C" int hook_symbol(const char* lib_name, const char* symbol_name, void* new_func, void** old_func) {
    HookContext ctx = { lib_name, symbol_name, new_func, old_func, false };
    dl_iterate_phdr(plt_hook_callback, &ctx);
    if (!ctx.hooked && old_func) {
        void* handle = dlopen(lib_name, RTLD_LAZY);
        if (handle) {
            *old_func = dlsym(handle, symbol_name);
        }
    }
    return ctx.hooked ? 0 : -1;
}

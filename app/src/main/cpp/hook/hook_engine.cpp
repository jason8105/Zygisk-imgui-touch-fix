#include "hook_engine.h"
#include <link.h>
#include <elf.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <android/log.h>

#define LOG_TAG "ZygiskHookEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace HookEngine {

void PltHookModule(uintptr_t base, const char* symbol_name, void* new_func, void** old_func) {
    if (!base) return;
    ElfW(Ehdr)* ehdr = (ElfW(Ehdr)*)base;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) return;

    ElfW(Phdr)* phdr = (ElfW(Phdr)*)(base + ehdr->e_phoff);
    ElfW(Dyn)* dyn = nullptr;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = (ElfW(Dyn)*)(base + phdr[i].p_vaddr);
            break;
        }
    }

    if (!dyn) return;

    const char* strtab = nullptr;
    ElfW(Sym)* symtab = nullptr;
    ElfW(Rela)* rela = nullptr;
    ElfW(Rel)* rel = nullptr;
    size_t relsz = 0;

    for (ElfW(Dyn)* d = dyn; d->d_tag != DT_NULL; d++) {
        switch (d->d_tag) {
            case DT_STRTAB: strtab = (const char*)(base + d->d_un.d_ptr); break;
            case DT_SYMTAB: symtab = (ElfW(Sym)*)(base + d->d_un.d_ptr); break;
            case DT_JMPREL: rela = (ElfW(Rela)*)(base + d->d_un.d_ptr); rel = (ElfW(Rel)*)(base + d->d_un.d_ptr); break;
            case DT_PLTRELSZ: relsz = d->d_un.d_val; break;
        }
    }

    if (!strtab || !symtab || (!rela && !rel)) return;

    size_t count = relsz / (rela ? sizeof(ElfW(Rela)) : sizeof(ElfW(Rel)));
    for (size_t i = 0; i < count; i++) {
        uintptr_t r_offset = rela ? rela[i].r_offset : rel[i].r_offset;
        uint32_t r_info = rela ? ELF64_R_SYM(rela[i].r_info) : ELF32_R_SYM(rel[i].r_info);

        const char* name = strtab + symtab[r_info].st_name;
        if (name && strcmp(name, symbol_name) == 0) {
            void** got_entry = (void**)(base + r_offset);
            if (*got_entry != new_func) {
                if (old_func && !*old_func) {
                    *old_func = *got_entry;
                }
                uintptr_t page_start = (uintptr_t)got_entry & ~((uintptr_t)getpagesize() - 1);
                mprotect((void*)page_start, getpagesize(), PROT_READ | PROT_WRITE);
                *got_entry = new_func;
                mprotect((void*)page_start, getpagesize(), PROT_READ);
                LOGI("Successfully PLT hooked %s -> %p", symbol_name, new_func);
            }
        }
    }
}

void PltHookAllModules(const char* symbol_name, void* new_func, void** old_func) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return;

    char line[512];
    uintptr_t prev_base = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "r-xp") || strstr(line, "r--p")) {
            uintptr_t base = 0;
            if (sscanf(line, "%lx-", &base) == 1) {
                if (base != prev_base) {
                    prev_base = base;
                    PltHookModule(base, symbol_name, new_func, old_func);
                }
            }
        }
    }
    fclose(fp);
}

} // namespace HookEngine

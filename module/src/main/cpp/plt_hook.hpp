#pragma once

#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "ZygiskTouch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace PltHook {

inline void hook_plt(const char* target_so, const char* symbol_name, void* hook_func, void** orig_func) {
    if (orig_func && !*orig_func) {
        *orig_func = dlsym(RTLD_DEFAULT, symbol_name);
    }

    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (target_so && !strstr(line, target_so)) continue;
        if (!strstr(line, "r-xp") && !strstr(line, "r--p") && !strstr(line, "rw-p")) continue;

        uintptr_t base_addr = 0;
        if (sscanf(line, "%lx-", &base_addr) != 1 || base_addr == 0) continue;

        ElfW(Ehdr)* ehdr = (ElfW(Ehdr)*)base_addr;
        if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) continue;

        ElfW(Phdr)* phdr = (ElfW(Phdr)*)(base_addr + ehdr->e_phoff);
        ElfW(Dyn)* dynamic = nullptr;

        for (int i = 0; i < ehdr->e_phnum; i++) {
            if (phdr[i].p_type == PT_DYNAMIC) {
                dynamic = (ElfW(Dyn)*)(base_addr + phdr[i].p_vaddr);
                break;
            }
        }
        if (!dynamic) continue;

        ElfW(Sym)* symtab = nullptr;
        const char* strtab = nullptr;
        ElfW(Rela)* rela = nullptr;
        size_t relasz = 0;
        ElfW(Rel)* rel = nullptr;
        size_t relsz = 0;

        for (ElfW(Dyn)* d = dynamic; d->d_tag != DT_NULL; ++d) {
            switch (d->d_tag) {
                case DT_STRTAB: strtab = (const char*)(base_addr + d->d_un.d_ptr); break;
                case DT_SYMTAB: symtab = (ElfW(Sym)*)(base_addr + d->d_un.d_ptr); break;
                case DT_JMPREL: rela = (ElfW(Rela)*)(base_addr + d->d_un.d_ptr); rel = (ElfW(Rel)*)(base_addr + d->d_un.d_ptr); break;
                case DT_PLTRELSZ: relasz = relsz = d->d_un.d_val; break;
            }
        }

        if (!strtab || !symtab) continue;

        size_t count = relasz / (rela ? sizeof(ElfW(Rela)) : sizeof(ElfW(Rel)));
        for (size_t i = 0; i < count; i++) {
            uintptr_t r_offset = 0;
            uint32_t r_info_sym = 0;
            if (rela) {
                r_offset = rela[i].r_offset;
                r_info_sym = ELF64_R_SYM(rela[i].r_info);
            } else if (rel) {
                r_offset = rel[i].r_offset;
                r_info_sym = ELF32_R_SYM(rel[i].r_info);
            }

            if (r_info_sym != 0) {
                const char* sym_name = strtab + symtab[r_info_sym].st_name;
                if (strcmp(sym_name, symbol_name) == 0) {
                    uintptr_t* got_entry = (uintptr_t*)(base_addr + r_offset);
                    if (*got_entry != (uintptr_t)hook_func) {
                        uintptr_t page_start = (uintptr_t)got_entry & ~((uintptr_t)PAGE_SIZE - 1);
                        mprotect((void*)page_start, PAGE_SIZE, PROT_READ | PROT_WRITE);
                        if (orig_func && !*orig_func) *orig_func = (void*)*got_entry;
                        *got_entry = (uintptr_t)hook_func;
                        mprotect((void*)page_start, PAGE_SIZE, PROT_READ);
                        LOGI("Hooked PLT symbol %s at address %p", symbol_name, got_entry);
                    }
                }
            }
        }
    }
    fclose(fp);
}

} // namespace PltHook

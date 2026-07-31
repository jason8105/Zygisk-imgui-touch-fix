#pragma once

#include <dlfcn.h>
#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <android/log.h>

#define LOG_TAG "ZygiskTouchMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace PltHook {

#if defined(__aarch64__) || defined(__x86_64__)
    typedef Elf64_Ehdr Elf_Ehdr;
    typedef Elf64_Phdr Elf_Phdr;
    typedef Elf64_Dyn Elf_Dyn;
    typedef Elf64_Sym Elf_Sym;
    typedef Elf64_Rela Elf_Rela;
    typedef Elf64_Rel Elf_Rel;
    #define ELF_R_SYM ELF64_R_SYM
#else
    typedef Elf32_Ehdr Elf_Ehdr;
    typedef Elf32_Phdr Elf_Phdr;
    typedef Elf32_Dyn Elf_Dyn;
    typedef Elf32_Sym Elf_Sym;
    typedef Elf32_Rel Elf_Rel;
    #define ELF_R_SYM ELF32_R_SYM
#endif

inline bool hook_symbol_in_library(const char* lib_name, const char* symbol_name, void* hook_func, void** orig_func) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return false;

    char line[512];
    uintptr_t base_addr = 0;
    bool found = false;

    while (fgets(line, sizeof(line), fp)) {
        if (lib_name == nullptr || strstr(line, lib_name)) {
            if (strstr(line, "r-xp") || strstr(line, "r--p") || strstr(line, "xp")) {
                uintptr_t start, end;
                if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
                    base_addr = start;
                    found = true;
                    break;
                }
            }
        }
    }
    fclose(fp);

    if (!found || !base_addr) return false;

    Elf_Ehdr* ehdr = (Elf_Ehdr*)base_addr;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) return false;

    Elf_Phdr* phdr = (Elf_Phdr*)(base_addr + ehdr->e_phoff);
    Elf_Dyn* dyn = nullptr;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = (Elf_Dyn*)(base_addr + phdr[i].p_vaddr);
            break;
        }
    }

    if (!dyn) return false;

    Elf_Sym* symtab = nullptr;
    const char* strtab = nullptr;
    Elf_Rel* rel = nullptr;
    Elf_Rela* rela = nullptr;
    size_t rel_sz = 0, rela_sz = 0;

    for (Elf_Dyn* d = dyn; d->d_tag != DT_NULL; ++d) {
        switch (d->d_tag) {
            case DT_SYMTAB: symtab = (Elf_Sym*)(base_addr + d->d_un.d_ptr); break;
            case DT_STRTAB: strtab = (const char*)(base_addr + d->d_un.d_ptr); break;
            case DT_REL: rel = (Elf_Rel*)(base_addr + d->d_un.d_ptr); break;
            case DT_RELSZ: rel_sz = d->d_un.d_val; break;
            case DT_RELA: rela = (Elf_Rela*)(base_addr + d->d_un.d_ptr); break;
            case DT_RELASZ: rela_sz = d->d_un.d_val; break;
        }
    }

    if (!symtab || !strtab) return false;

    bool hooked = false;

    auto process_rel = [&](uintptr_t rel_addr, size_t size, bool is_rela) {
        size_t count = size / (is_rela ? sizeof(Elf_Rela) : sizeof(Elf_Rel));
        for (size_t i = 0; i < count; i++) {
            uintptr_t r_offset = 0;
            size_t r_sym = 0;

            if (is_rela) {
                Elf_Rela* ra = (Elf_Rela*)rel_addr + i;
                r_offset = ra->r_offset;
                r_sym = ELF_R_SYM(ra->r_info);
            } else {
                Elf_Rel* r = (Elf_Rel*)rel_addr + i;
                r_offset = r->r_offset;
                r_sym = ELF_R_SYM(r->r_info);
            }

            if (r_sym == 0) continue;
            const char* name = strtab + symtab[r_sym].st_name;
            if (name && strcmp(name, symbol_name) == 0) {
                uintptr_t* target = (uintptr_t*)(base_addr + r_offset);
                if (*target != (uintptr_t)hook_func) {
                    if (orig_func && *orig_func == nullptr) {
                        *orig_func = (void*)*target;
                    }
                    uintptr_t page_size = sysconf(_SC_PAGESIZE);
                    uintptr_t page_start = (uintptr_t)target & ~(page_size - 1);
                    mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE);
                    *target = (uintptr_t)hook_func;
                    mprotect((void*)page_start, page_size, PROT_READ);
                    hooked = true;
                    LOGI("Successfully PLT-hooked %s in %s", symbol_name, lib_name ? lib_name : "loaded_so");
                }
            }
        }
    };

    if (rel && rel_sz) process_rel((uintptr_t)rel, rel_sz, false);
    if (rela && rela_sz) process_rel((uintptr_t)rela, rela_sz, true);

    return hooked;
}

inline void hook_all_modules(const char* symbol_name, void* hook_func, void** orig_func) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return;

    char line[512];
    std::vector<std::string> libs;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, ".so") && (strstr(line, "r-xp") || strstr(line, "xp"))) {
            char path[256];
            if (sscanf(line, "%*s %*s %*s %*s %*s %255s", path) == 1) {
                bool exists = false;
                for (const auto& l : libs) {
                    if (l == path) { exists = true; break; }
                }
                if (!exists) libs.push_back(path);
            }
        }
    }
    fclose(fp);

    for (const auto& lib : libs) {
        hook_symbol_in_library(lib.c_str(), symbol_name, hook_func, orig_func);
    }
}

} // namespace PltHook

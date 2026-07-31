#include "hook_utils.hpp"
#include <link.h>
#include <elf.h>
#include <sys/mman.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "HookUtils"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace HookUtils {

bool HookSymbol(const char* lib_pattern, const char* symbol_name, void* new_func, void** old_func) {
    void* target_addr = dlsym(RTLD_DEFAULT, symbol_name);
    if (!target_addr) {
        LOGE("Symbol %s not found in process", symbol_name);
        return false;
    }
    if (old_func && !*old_func) {
        *old_func = target_addr;
    }

    struct ScanCtx {
        const char* lib_pattern;
        void* target_addr;
        void* new_func;
        bool found;
    } ctx = { lib_pattern, target_addr, new_func, false };

    dl_iterate_phdr([](struct dl_phdr_info *info, size_t, void *data) -> int {
        auto* c = reinterpret_cast<ScanCtx*>(data);
        if (!info->dlpi_name) return 0;

        if (c->lib_pattern && strlen(c->lib_pattern) > 0) {
            if (strstr(info->dlpi_name, c->lib_pattern) == nullptr) {
                return 0;
            }
        }

        uintptr_t page_size = sysconf(_SC_PAGESIZE);
        for (int i = 0; i < info->dlpi_phnum; i++) {
            if (info->dlpi_phdr[i].p_type == PT_LOAD && (info->dlpi_phdr[i].p_flags & PF_W || info->dlpi_phdr[i].p_flags & PF_R)) {
                uintptr_t seg_start = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;
                uintptr_t seg_end = seg_start + info->dlpi_phdr[i].p_memsz;
                
                void** ptr = (void**)seg_start;
                void** end = (void**)seg_end;
                
                while (ptr < end) {
                    if (*ptr == c->target_addr) {
                        uintptr_t page_start = (uintptr_t)ptr & ~(page_size - 1);
                        mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE);
                        *ptr = c->new_func;
                        mprotect((void*)page_start, page_size, PROT_READ);
                        c->found = true;
                        LOGI("Hooked symbol at %p in %s", ptr, info->dlpi_name);
                    }
                    ptr++;
                }
            }
        }
        return 0;
    }, &ctx);

    return ctx.found;
}

} // namespace HookUtils

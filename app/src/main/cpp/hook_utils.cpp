#include "hook_utils.hpp"
#include <android/log.h>

#define LOG_TAG "ZygiskImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace NativeHook {

static uintptr_t GetPageStart(uintptr_t addr) {
    return addr & PAGE_MASK;
}

static bool UnprotectPage(void* addr) {
    uintptr_t page_start = GetPageStart(reinterpret_cast<uintptr_t>(addr));
    return mprotect(reinterpret_cast<void*>(page_start), PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

struct PhdrCallbackArgs {
    const char* symbol_name;
    void* target_func;
    void* replace_func;
    int patched_count;
};

static int PhdrCallback(struct dl_phdr_info* info, size_size_t /*unused*/, void* data) {
    PhdrCallbackArgs* args = reinterpret_cast<PhdrCallbackArgs*>(data);
    if (!info || !args->target_func) return 0;

    for (int i = 0; i < info->dlpi_phnum; ++i) {
        if (info->dlpi_phdr[i].p_type == PT_DYNAMIC) {
            ElfW(Dyn)* dyn = reinterpret_cast<ElfW(Dyn)*>(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);
            ElfW(Addr)* got = nullptr;

            for (ElfW(Dyn)* d = dyn; d->d_tag != DT_NULL; ++d) {
                if (d->d_tag == DT_PLTGOT) {
                    got = reinterpret_cast<ElfW(Addr)*>(d->d_un.d_ptr);
                    break;
                }
            }

            if (got) {
                for (int j = 0; j < 1024; ++j) {
                    void** got_entry = reinterpret_cast<void**>(&got[j]);
                    if (*got_entry == args->target_func) {
                        if (UnprotectPage(got_entry)) {
                            *got_entry = args->replace_func;
                            args->patched_count++;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

bool HookAll(const char* symbol_name, void* replace_func, void** orig_func) {
    void* target = dlsym(RTLD_DEFAULT, symbol_name);
    if (!target) {
        LOGI("Symbol %s not found", symbol_name);
        return false;
    }

    if (orig_func && !*orig_func) {
        *orig_func = target;
    }

    PhdrCallbackArgs args{symbol_name, target, replace_func, 0};
    dl_iterate_phdr(PhdrCallback, &args);
    LOGI("Hooked symbol %s at %d location(s)", symbol_name, args.patched_count);
    return true;
}

}

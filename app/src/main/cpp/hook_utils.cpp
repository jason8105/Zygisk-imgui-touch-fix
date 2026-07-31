#include "hook_utils.h"
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "ZygiskHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void* find_library_symbol(const char* lib_name, const char* symbol_name) {
    void* handle = dlopen(lib_name, RTLD_NOW);
    if (!handle) {
        return nullptr;
    }
    void* sym = dlsym(handle, symbol_name);
    return sym;
}

bool plt_hook(const char* lib_name, const char* symbol_name, void* replace_fn, void** orig_fn) {
    void* sym = find_library_symbol(lib_name, symbol_name);
    if (sym && orig_fn) {
        *orig_fn = sym;
        return true;
    }
    return false;
}

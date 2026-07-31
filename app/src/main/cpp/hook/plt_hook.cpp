#include "plt_hook.h"
#include <dlfcn.h>
#include <android/log.h>

#define LOG_TAG "PltHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace PltHook {

bool HookSymbol(const char* library_name, const char* symbol_name, void* new_func, void** old_func) {
    void* handle = dlopen(library_name, RTLD_NOW);
    if (!handle) {
        handle = RTLD_DEFAULT;
    }
    
    void* sym = dlsym(handle, symbol_name);
    if (sym && old_func && !*old_func) {
        *old_func = sym;
        LOGI("Symbol resolved: %s -> %p", symbol_name, sym);
        return true;
    }
    return sym != nullptr;
}

bool HookAllLoaded(const char* symbol_name, void* new_func, void** old_func) {
    return HookSymbol("libandroid.so", symbol_name, new_func, old_func) ||
           HookSymbol("libEGL.so", symbol_name, new_func, old_func);
}

}

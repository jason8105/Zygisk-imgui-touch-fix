#include "hook_utils.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

#define LOG_TAG "ZygiskHookUtils"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace HookUtils {

void* GetModuleBase(const char* moduleName) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return nullptr;
    
    char line[512];
    uintptr_t base = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, moduleName)) {
            base = strtoul(line, nullptr, 16);
            break;
        }
    }
    fclose(fp);
    return reinterpret_cast<void*>(base);
}

bool HookSymbol(const char* libName, const char* symbolName, void* hookFunc, void** origFunc) {
    void* handle = dlopen(libName, RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) {
        handle = dlopen(libName, RTLD_NOW);
    }
    if (!handle) return false;

    void* sym = dlsym(handle, symbolName);
    if (!sym) return false;

    if (origFunc) {
        *origFunc = sym;
    }
    return true;
}

}

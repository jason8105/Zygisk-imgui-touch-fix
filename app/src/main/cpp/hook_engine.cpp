#include "hook_engine.h"
#include <dlfcn.h>
#include <android/log.h>

extern "C" {
    __attribute__((weak)) int DobbyHook(void *function_address, void *replace_call, void **origin_call);
}

#define LOG_TAG "ZygiskImGuiHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace HookEngine {

bool Hook(void *target, void *replace, void **origin) {
    if (!target || !replace) return false;

    if (DobbyHook) {
        int res = DobbyHook(target, replace, origin);
        if (res == 0) {
            LOGI("Successfully hooked function at %p", target);
            return true;
        }
    }

    LOGE("Failed to hook target at %p (Dobby unavailable or error)", target);
    return false;
}

bool HookSymbol(const char *library_name, const char *symbol_name, void *replace, void **origin) {
    void *handle = RTLD_DEFAULT;
    if (library_name) {
        handle = dlopen(library_name, RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            LOGE("Failed to load library %s: %s", library_name, dlerror());
            return false;
        }
    }

    void *symbol = dlsym(handle, symbol_name);
    if (!symbol) {
        LOGE("Failed to resolve symbol %s", symbol_name);
        return false;
    }

    return Hook(symbol, replace, origin);
}

}

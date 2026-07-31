#pragma once

#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

namespace NativeHook {
    bool HookAll(const char* symbol_name, void* replace_func, void** orig_func);
}

#ifndef PLT_HOOK_H
#ifndef PLT_HOOK_H
#define PLT_HOOK_H

namespace PLTHook {
    void RegisterHook(const char* symbol_name, void* new_func, void** old_func);
    void ApplyHooks();
}

#endif // PLT_HOOK_H

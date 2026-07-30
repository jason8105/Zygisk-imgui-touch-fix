#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

namespace zygisk {

class Api {
public:
    enum State {
        DLOPEN_FAILED = 0,
        OK = 1
    };

    enum Option {
        // Option to strip or keep companion communication
        FORCE_DENYLIST_UNMOUNT = 1
    };

    virtual void *JNIEnv() = 0;
    virtual void *GetModuleInfo() = 0;
    virtual void *ConnectCompanion() = 0;
    virtual void HookJniNativeMethods(void *env, const char *className, void *methods, int numMethods) = 0;
    virtual int HookDlopen(const char *name, void *callback) = 0;
    virtual void *Aborter() = 0;
    virtual void SetOption(Option opt) = 0;
};

class Module {
public:
    virtual void OnLoad(Api *api, JNIEnv *env) {}
    virtual void PreAppSpecialize(Api *api, void *specializeArgs) {}
    virtual void PostAppSpecialize(Api *api, void *specializeArgs) {}
    virtual void PreServerSpecialize(Api *api, void *specializeArgs) {}
    virtual void PostServerSpecialize(Api *api, void *specializeArgs) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        static clazz module; \
        module.OnLoad(api, env); \
    }

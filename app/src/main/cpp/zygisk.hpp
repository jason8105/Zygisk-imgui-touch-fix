#pragma once

#include <stddef.h>
#include <stdint.h>

namespace zygisk {

enum ApiVersion {
    SUPPORTED_API_VERSION = 1
};

class AppSpecializeArgs {
public:
    int32_t &uid;
    int32_t &gid;
    // other fields omitted for brevity
};

class ServerSpecializeArgs {
public:
    int32_t &uid;
    int32_t &gid;
};

class ZygiskApi {
public:
    enum Option {
        DLCLOSE_MODULE_LIBRARY = 0
    };

    virtual void *val(Option opt) = 0;
    virtual void plt_hook_register(const char *lib_name, const char *symbol, void *new_func, void **old_func) = 0;
    virtual bool plt_hook_commit() = 0;
    virtual void *internal_dlopen(const char *file, int flags) = 0;
    virtual void set_unload_policy(int policy) = 0;
};

class ZygiskModule {
public:
    virtual ~ZygiskModule() {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

typedef void (*ModuleABIEntry)(ZygiskApi *api, int version);

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    extern "C" __attribute__((visibility("default"))) __attribute__((used)) \
    void zygisk_module_entry(zygisk::ZygiskApi *api, int version) { \
        static clazz module; \
        api->set_unload_policy(0); \
        /* initialization */ \
    }

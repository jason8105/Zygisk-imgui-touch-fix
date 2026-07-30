#pragma once

#include <stdint.h>
#include <stddef.h>

namespace zygisk {

enum ApiVersion : uint32_t {
    V1 = 1,
};

enum Option : uint32_t {
    DLCLOSE_MODULE_LIBRARY = 0,
};

class Api {
public:
    virtual void *JNIEnv() = 0;
    virtual void setOption(Option opt) = 0;
    virtual int hook_plt(const char *lib_name, const char *symbol, void *new_func, void **old_func) = 0;
    virtual int plt_hook_register(const char *lib_name, const char *symbol, void *new_func, void **old_func) = 0;
    virtual bool plt_hook_commit() = 0;
    virtual void *somap_get_symbol(const char *lib_name, const char *symbol) = 0;
    virtual void *connect_companion() = 0;
};

class AppSpecializeArgs {
public:
    uint64_t uid;
    uint64_t gid;
    const char *gids;
    int rt_capabilities;
    const char *selinux_context;
    const char *package_name;
    const char *app_data_dir;
    bool is_child_zygote;
    // other fields omitted for compatibility
};

class ServerSpecializeArgs {
public:
    uint64_t uid;
    uint64_t gid;
    const char *gids;
    int rt_capabilities;
    // other fields omitted for compatibility
};

class ModuleBase {
public:
    virtual ~ModuleBase() = default;
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    static void *zygisk_module_instance = nullptr; \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        static_cast<void>(env); \
        zygisk_module_instance = new clazz(); \
        api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY); \
    }

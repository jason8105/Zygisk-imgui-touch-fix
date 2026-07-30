#ifndef ZYGISK_HPP
#define ZYGISK_HPP

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

struct AppSpecializeArgs {
    JNIEnv *env;
    jint *uid;
    jint *gid;
    jint **gids;
    jint *rt_nice;
    jobjectArray *seclabel;
    jstring *app_data_dir;
    jint *inet;
    jint *outer_appid;
    jint *is_isolated;
    jobjectArray *pkg_name;
    jstring *hosting_type;
    jstring *hosting_name_is_service;
    jint *disabled_modules;
};

struct ServerSpecializeArgs {
    JNIEnv *env;
    jint *uid;
    jint *gid;
    jint **gids;
    jint *rt_nice;
    jobjectArray *seclabel;
};

class Api {
public:
    enum Flag {
        overlay_dlclose = (1 << 0)
    };
    
    virtual void *dlopen(const char *filename, int flag) = 0;
    virtual void *dlsym(void *handle, const char *symbol) = 0;
    virtual void plt_hook_register(const char *lib_name, const char *symbol, void *new_func, void **old_func) = 0;
    virtual bool plt_hook_commit() = 0;
    virtual int ez_hook(const char *lib_name, const char *symbol, void *new_func, void **old_func) = 0;
    virtual void set_shaded_storage() = 0;
    virtual void *connect_companion() = 0;
};

class ModuleBase {
public:
    virtual ~ModuleBase() {}
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(ServerSpecializeArgs *args) {}
};

} // namespace zygisk

#define ZYGISK_MODULE_ENTRY(module_class) \
    static module_class __zygisk_module; \
    extern "C" { \
        ABI_EXPORT void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
            __zygisk_module.onLoad(api, env); \
        } \
        ABI_EXPORT void zygisk_companion_entry(int socket) {} \
    }

#ifndef ABI_EXPORT
#define ABI_EXPORT __attribute__((visibility("default")))
#endif

#endif // ZYGISK_HPP

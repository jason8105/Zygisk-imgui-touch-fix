#ifndef ZYGISK_HPP
#define ZYGISK_HPP

#include <jni.h>
#include <sys/types.h>

namespace zygisk {

struct AppSpecializeArgs {
    jint *uid;
    jint *gid;
    jobjectArray *gids;
    jint *runtime_flags;
    jobjectArray *zygisk_abi_list;
    jstring *instruction_set;
    jstring *app_data_dir;
};

struct ServerSpecializeArgs {
    jint *uid;
    jint *gid;
    jobjectArray *gids;
    jint *runtime_flags;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

class Api {
public:
    enum Option {
        FEATURE_DLCLOSE_MODS = 0,
        FORCE_DENYLIST_UNMOUNT = 1
    };
    virtual void setOption(Option opt) = 0;
    virtual int hook_plt(const char *lib_name, const char *symbol, void *new_func, void **old_func) = 0;
    virtual void *plt_safe_hook_symbol(const char *lib_name, const char *symbol, void *new_func) = 0;
    virtual void inner_dlclose(void *handle) = 0;
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

#define REGISTER_ZYGISK_MODULE(clazz) \
    static clazz __zygisk_module; \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        __zygisk_module.onLoad(api, env); \
    } \
    extern "C" __attribute__((visibility("default"))) void zygisk_companion_entry(int client) { \
    }

#endif // ZYGISK_HPP

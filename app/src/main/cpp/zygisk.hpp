#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

struct Api;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

struct AppSpecializeArgs {
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *rlimits;
    jint *mount_external;
    jstring *se_info;
    jstring *nice_name;
    jintArray *is_child_zygote;
    jstring *instruction_set;
    jstring *app_data_dir;
    jboolean *is_top_app;
    jobjectArray *pkg_data_info_list;
    jobjectArray *whitelisted_data_info_list;
    jboolean *mount_data_dirs;
    jboolean *mount_storage_dirs;
};

struct ServerSpecializeArgs {
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *rlimits;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

enum Option : uint32_t {
    DLCLOSE_MODULE_LIBRARY = 0,
    FORCE_DENYLIST_UNMOUNT = 1,
};

struct Api {
    void *impl;
    bool connectCompanion();
    int getCompanionSocket();
    void setOption(Option opt);
};

typedef void (*RegisterModuleFn)(Api *, JNIEnv *);

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
static void _zygisk_register_module(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    module.onLoad(api, env); \
} \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    _zygisk_register_module(api, env); \
}

#define REGISTER_ZYGISK_COMPANION(func) \
extern "C" [[gnu::visibility("default")]] \
void zygisk_companion_entry(int socket) { \
    func(socket); \
}

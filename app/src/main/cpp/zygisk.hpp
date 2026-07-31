#pragma once

#include <jni.h>
#include <stdint.h>
#include <sys/types.h>

namespace zygisk {

struct AppSpecializeArgs {
    uid_t *uid;
    gid_t *gid;
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
    uid_t *uid;
    gid_t *gid;
    jintArray *gids;
    jint *runtime_flags;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

enum Option : uint32_t {
    FORCE_DENYLIST_UNLOAD = 0,
    DLCLOSE_MODULE_FOR_UNOBSERVED_PROCESS = 1,
};

class Api {
public:
    virtual void setOption(Option opt) = 0;
    virtual int getModuleDir() = 0;
    virtual bool exemptAppProcess() = 0;
    virtual void pltHookRegister(dev_t dev, ino_t inode, const char *symbol, void *new_func, void **old_func) = 0;
    virtual bool pltHookCommit() = 0;
    virtual void connectCompanion() = 0;
    virtual int getCompanionSocket() = 0;
};

class Module {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
static clazz _instance; \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    _instance.onLoad(api, env); \
} \
extern "C" [[gnu::visibility("default")]] \
void zygisk_pre_app_specialize(zygisk::AppSpecializeArgs *args) { \
    _instance.preAppSpecialize(args); \
} \
extern "C" [[gnu::visibility("default")]] \
void zygisk_post_app_specialize(const zygisk::AppSpecializeArgs *args) { \
    _instance.postAppSpecialize(args); \
} \
extern "C" [[gnu::visibility("default")]] \
void zygisk_pre_server_specialize(zygisk::ServerSpecializeArgs *args) { \
    _instance.preServerSpecialize(args); \
} \
extern "C" [[gnu::visibility("default")]] \
void zygisk_post_server_specialize(const zygisk::ServerSpecializeArgs *args) { \
    _instance.postServerSpecialize(args); \
}

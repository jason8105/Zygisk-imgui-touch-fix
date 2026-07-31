#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

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
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

class Api {
public:
    virtual void connectCompanion() = 0;
    virtual int getCompanionFd() = 0;
    virtual void setOption(int option) = 0;
    virtual void moduleLoaded() = 0;
    virtual void pltHookRegister(dev_t dev, ino_t inode, const char *symbol, void *new_func, void **old_func) = 0;
    virtual bool pltHookCommit() = 0;
    virtual void exemptFd(int fd) = 0;
};

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
static void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    module.onLoad(api, env); \
} \
extern "C" __attribute__((visibility("default"))) \
void zygisk_module_entry_v1(zygisk::Api *api, JNIEnv *env) { \
    zygisk_module_entry(api, env); \
}

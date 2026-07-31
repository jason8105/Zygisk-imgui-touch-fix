#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

enum Option : uint32_t {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_LIBRARY = 1,
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
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

class Api {
public:
    virtual void connectCompanion() = 0;
    virtual int getModuleDir() = 0;
    virtual void setOption(Option option) = 0;
    virtual void exemptAppProcess() = 0;
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

extern "C" {
typedef void (*zygisk_module_entry_t)(zygisk::Api *, JNIEnv *);
}

#define REGISTER_ZYGISK_MODULE(clazz) \
static void zygisk_module_entry_impl(zygisk::Api *api, JNIEnv *env) { \
    static clazz instance; \
    static zygisk::ModuleBase *module = &instance; \
    module->onLoad(api, env); \
} \
extern "C" [[gnu::visibility("default")]] zygisk_module_entry_t zygisk_module_entry = zygisk_module_entry_impl;

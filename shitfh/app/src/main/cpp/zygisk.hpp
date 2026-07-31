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
    jobjectArray *rlimits;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

enum Option : uint32_t {
    DLCLOSE_MODULE_LIBRARY = 1 << 0,
    FORCE_DENYLIST_UNMOUNT = 1 << 1,
};

class Api;
class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

class Api {
public:
    void *impl;
    void connectCompanion();
    int getCompanionSocket();
    void setOption(Option opt);
    bool exemptAppEnv();
};

} // namespace zygisk

extern "C" {
    void zygisk_module_entry(zygisk::Api *api, JNIEnv *env);
}

#define REGISTER_ZYGISK_MODULE(clazz) \
static clazz _zygisk_module_instance; \
extern "C" void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    _zygisk_module_instance.onLoad(api, env); \
}

#pragma once

#include <jni.h>
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

enum Option : uint32_t {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_FOR_UNLOADED_PROCESS = 1,
};

class Api {
public:
    virtual bool connectCompanion() = 0;
    virtual int getCompanionSocket() = 0;
    virtual void setOption(Option opt) = 0;
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
static clazz _zygisk_module_instance; \
extern "C" __attribute__((visibility("default"))) \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    _zygisk_module_instance.onLoad(api, env); \
} \
extern "C" __attribute__((visibility("default"))) \
zygisk::ModuleBase* zygisk_companion_entry() { \
    return nullptr; \
}

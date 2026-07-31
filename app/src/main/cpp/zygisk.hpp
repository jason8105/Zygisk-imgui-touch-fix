#pragma once

#include <jni.h>
#include <stdint.h>

namespace zygisk {

class Api;
class AppSpecializeArgs;
class ServerSpecializeArgs;

enum Option {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_LIBRARY = 1,
};

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
    virtual bool connectCompanion() = 0;
    virtual int getCompanionSocket() = 0;
    virtual void setOption(Option opt) = 0;
    virtual void exemptFd(int fd) = 0;
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

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
static void _zygisk_init(zygisk::Api *api, JNIEnv *env) { \
    static clazz instance; \
    zygisk::ModuleBase *module = &instance; \
    module->onLoad(api, env); \
} \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    _zygisk_init(api, env); \
}

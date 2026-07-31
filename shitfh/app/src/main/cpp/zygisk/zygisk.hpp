#pragma once

#include <jni.h>
#include <stddef.h>

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
};

struct ServerSpecializeArgs {
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

enum Option {
    DLCLOSE_MODULE_FOR_UNLOADED_PROCESS = 0,
    FORCE_DENYLIST_UNMOUNT = 1,
};

class Api {
public:
    virtual void connectCompanion() = 0;
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

typedef void (*RegisterModuleFn)(Api *, JNIEnv *);

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
static void zygisk_module_init(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    static bool initialized = false; \
    if (!initialized) { \
        initialized = true; \
        module.onLoad(api, env); \
    } \
} \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    zygisk_module_init(api, env); \
}

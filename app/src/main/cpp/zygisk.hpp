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

enum Option : int {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_FOR_UNLOADED_PROCESS = 1,
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
    virtual void connectCompanion() = 0;
    virtual int getCompanionSocket() = 0;
    virtual bool setOption(Option opt) = 0;
};

namespace internal {
struct module_abi {
    Api *api;
    ModuleBase *module;
    void (*preAppSpecialize)(ModuleBase *, AppSpecializeArgs *);
    void (*postAppSpecialize)(ModuleBase *, const AppSpecializeArgs *);
    void (*preServerSpecialize)(ModuleBase *, ServerSpecializeArgs *);
    void (*postServerSpecialize)(ModuleBase *, const ServerSpecializeArgs *);
};
}

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    static zygisk::internal::module_abi abi{ \
        api, \
        &module, \
        [](zygisk::ModuleBase *m, zygisk::AppSpecializeArgs *args) { m->preAppSpecialize(args); }, \
        [](zygisk::ModuleBase *m, const zygisk::AppSpecializeArgs *args) { m->postAppSpecialize(args); }, \
        [](zygisk::ModuleBase *m, zygisk::ServerSpecializeArgs *args) { m->preServerSpecialize(args); }, \
        [](zygisk::ModuleBase *m, const zygisk::ServerSpecializeArgs *args) { m->postServerSpecialize(args); }, \
    }; \
    module.onLoad(api, env); \
}

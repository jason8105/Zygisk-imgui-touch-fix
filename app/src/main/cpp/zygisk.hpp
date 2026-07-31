#pragma once

#include <jni.h>
#include <stdint.h>
#include <stddef.h>

namespace zygisk {

class Option {
public:
    enum : uint32_t {
        FORCE_DENYLIST_UNMOUNT = 1 << 0,
        DLCLOSE_MODULE_LIBRARY = 1 << 1,
    };
};

class AppSpecializeArgs {
public:
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

class ServerSpecializeArgs {
public:
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *rlimits;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

class Api {
public:
    virtual void connectCompanion() = 0;
    virtual int getModuleDir() = 0;
    virtual void setOption(Option option) = 0;
};

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    zygisk::ModuleBase *m = &module; \
    m->onLoad(api, env); \
}

} // namespace zygisk

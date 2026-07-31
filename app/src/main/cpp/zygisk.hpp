#pragma once

#include <jni.h>
#include <stdint.h>
#include <sys/types.h>

namespace zygisk {

class Api;
class AppSpec;
class ServerUtils;

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpec *spec) {}
    virtual void postAppSpecialize(const AppSpec *spec) {}
    virtual void preServerSpecialize(ServerUtils *utils) {}
    virtual void postServerSpecialize(const ServerUtils *utils) {}
};

enum Option : uint32_t {
    FORCE_DENYLIST_UNMOUNT = 1 << 0,
    DLCLOSE_MODULE_LIBRARY = 1 << 1,
};

class Api {
public:
    virtual bool connectCompanion() = 0;
    virtual void setOption(Option opt) = 0;
    virtual void pltHookRegister(dev_t dev, ino_t inode, const char *symbol, void *new_func, void **old_func) = 0;
    virtual bool pltHookCommit() = 0;
    virtual void exemptAppProcess() = 0;
};

class AppSpec {
public:
    jint uid;
    jint gid;
    jintArray gids;
    jint runtime_flags;
    jobjectArray rlimits;
    jint mount_external;
    jstring seinfo;
    jstring nice_name;
    jintArray is_child_zygote;
    jstring instruction_set;
    jstring app_data_dir;
    jboolean is_top_app;
    jobjectArray pkg_data_info_list;
    jobjectArray whitelisted_data_info_list;
    jboolean mount_data_dirs;
    jboolean mount_storage_dirs;
};

class ServerUtils {
public:
    virtual void exemptServerProcess() = 0;
};

} // namespace zygisk

extern "C" {
    void zygisk_module_entry(zygisk::Api *api, JNIEnv *env);
}

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY); \
    module.onLoad(api, env); \
}

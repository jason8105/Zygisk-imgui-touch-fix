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
    jobjectArray *rlimits;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

enum Option : uint32_t {
    DLCLOSE_MODULE_AFTER_POST_SPECIALIZE = 0,
    FORCE_DENYLIST_UNMOUNT = 1,
};

class Api {
public:
    virtual void connectCompanion() = 0;
    virtual int getCompanionFd() = 0;
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

extern "C" {
struct zygisk_module_api {
    void *impl;
    void (*onLoad)(void *, zygisk::Api *, JNIEnv *);
    void (*preAppSpecialize)(void *, zygisk::AppSpecializeArgs *);
    void (*postAppSpecialize)(void *, const zygisk::AppSpecializeArgs *);
    void (*preServerSpecialize)(void *, zygisk::ServerSpecializeArgs *);
    void (*postServerSpecialize)(void *, const zygisk::ServerSpecializeArgs *);
};
}

#define REGISTER_ZYGISK_MODULE(clazz) \
static clazz _zygisk_module_inst; \
static void _zygisk_onLoad(void *impl, zygisk::Api *api, JNIEnv *env) { \
    reinterpret_cast<clazz*>(impl)->onLoad(api, env); \
} \
static void _zygisk_preAppSpecialize(void *impl, zygisk::AppSpecializeArgs *args) { \
    reinterpret_cast<clazz*>(impl)->preAppSpecialize(args); \
} \
static void _zygisk_postAppSpecialize(void *impl, const zygisk::AppSpecializeArgs *args) { \
    reinterpret_cast<clazz*>(impl)->postAppSpecialize(args); \
} \
static void _zygisk_preServerSpecialize(void *impl, zygisk::ServerSpecializeArgs *args) { \
    reinterpret_cast<clazz*>(impl)->preServerSpecialize(args); \
} \
static void _zygisk_postServerSpecialize(void *impl, const zygisk::ServerSpecializeArgs *args) { \
    reinterpret_cast<clazz*>(impl)->postServerSpecialize(args); \
} \
extern "C" [[gnu::visibility("default")]] \
zygisk_module_api zygisk_module_entry = { \
    &_zygisk_module_inst, \
    _zygisk_onLoad, \
    _zygisk_preAppSpecialize, \
    _zygisk_postAppSpecialize, \
    _zygisk_preServerSpecialize, \
    _zygisk_postServerSpecialize \
};

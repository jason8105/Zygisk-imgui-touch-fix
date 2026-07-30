#pragma once
#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

enum ApiVersion {
    v1 = 1,
    v2 = 2,
    v3 = 3,
    v4 = 4
};

enum Option {
    // Daemon connection state
    FORCE_DENYLIST_UNMOUNT = 1
};

class AppSpecializeArgs {
public:
    jint *uid;
    jint *gid;
    jobjectArray *gids;
    jint *runtimeFlags;
    jobjectArray *seInfo;
    jobjectArray *name;
    jstring *deviceCode;
    jstring *niceName;
    jstring *workDir;
    jobjectArray *pkgDataInfoList;
    jobjectArray *whitelistedDataInfoList;
    jboolean *skipAppProfile;
    jstring *instructionSet;
    jstring *appDataDir;
};

class ServerSpecializeArgs {
public:
    jint *uid;
    jint *gid;
    jobjectArray *gids;
    jint *runtimeFlags;
    jobjectArray *seInfo;
    jobjectArray *niceName;
};

class Api {
public:
    virtual ~Api() = default;
    virtual void *pltHookRegister(const char *lib_name, const char *symbol, void *new_func, void **old_func) = 0;
    virtual void pltHookCommit() = 0;
    virtual int moduleGetDataDir() = 0;
    virtual void setOption(Option opt) = 0;
    virtual int connectCompanion() = 0;
};

class Module {
public:
    virtual ~Module() = default;
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    static clazz _zygisk_module_instance; \
    extern "C" { \
        __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
            _zygisk_module_instance.onLoad(api, env); \
        } \
        __attribute__((visibility("default"))) void zygisk_api_version(int *version) { \
            *version = 4; \
        } \
    }

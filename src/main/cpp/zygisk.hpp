#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

enum ApiVersion {
    v1 = 1
};

class Api {
public:
    virtual int GetDataDir() = 0;
    virtual void *GetServerForgeSocket() = 0;
    virtual void *GetModuleDir() = 0;
    virtual void MmapExec(void *&addr, size_t &length) = 0;
    virtual void HookJniNativeMethods(JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) = 0;
    virtual void plt_hook_register(const char *lib_name, const char *symbol_name, void *new_func, void **old_func) = 0;
    virtual bool plt_hook_commit() = 0;
    virtual int xopen_replacement(const char *pathname, int flags, int mode) = 0;
};

class AppSpecializeArgs {
public:
    JNIEnv *env;
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *seinfo;
    jobjectArray *nice_name;
    jboolean *is_child_zygote;
    jobjectArray *instruction_set;
    jobjectArray *app_data_dir;
};

class ServerSpecializeArgs {
public:
    JNIEnv *env;
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *seinfo;
    jobjectArray *nice_name;
    jboolean *is_child_zygote;
    jobjectArray *app_data_dir;
};

class ModuleBase {
public:
    virtual ~ModuleBase() {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(ServerSpecializeArgs *args) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    static void *zygisk_module_instance = nullptr; \
    extern "C" __attribute__((visibility("default"))) __attribute__((used)) \
    void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        static clazz module; \
        zygisk_module_instance = &module; \
    }

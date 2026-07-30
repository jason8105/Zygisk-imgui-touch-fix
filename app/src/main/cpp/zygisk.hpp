#pragma once
#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

enum ApiVersion {
    V1 = 1
};

class Api {
public:
    virtual int64_t getModuleVersion() = 0;
    virtual void *pltHookRegister(const char *soname, const char *symbol, void *new_func, void **old_func) = 0;
    virtual void *inlineHookRegister(void *symbol, void *new_func, void **old_func) = 0;
    virtual void showToast(const char *message) = 0;
    virtual int zc_disable_zygote_security() = 0;
    virtual void *connOnModuleLoaded(const char *name, void *arg) = 0;
    virtual void *connectCompanion() = 0;
};

class AppSpecializeArgs {
public:
};

class ServerSpecializeArgs {
public:
};

class ModuleBase {
public:
    virtual void onPreAppSpecialize(AppSpecializeArgs *args) {}
    virtual void onPostAppSpecialize(AppSpecializeArgs *args) {}
    virtual void onPreServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void onPostServerSpecialize(ServerSpecializeArgs *args) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    static void *zygisk_module_init_impl(zygisk::Api *api, JNIEnv *env) { \
        auto *mod = new clazz(); \
        mod->init(api, env); \
        return mod; \
    } \
    extern "C" __attribute__((visibility("default"))) __attribute__((used)) \
    void zygisk_module_init(zygisk::Api *api, JNIEnv *env) { \
        zygisk_module_init_impl(api, env); \
    }

#pragma once

#include <jni.h>

namespace zygisk {

struct Api;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;

enum RioCommand {
    // Commands placeholder for Magisk 24-26
};

class ModuleBase {
public:
    virtual ~ModuleBase() = default;
    virtual void onLoad(Api* api, JNIEnv* env) {}
    virtual void preAppSpecialize(AppSpecializeArgs* args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs* args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs* args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs* args) {}
};

enum ApiLevel {
    v1 = 1,
    v2 = 2,
    v3 = 3,
};

struct Api {
    virtual void* pltProtect(int, void*, size_t) = nullptr;
    virtual void* pltHook(int, const char*, void*, void**) = nullptr;
    virtual int option(int, ...) = nullptr;
    virtual void* getModulePath() = nullptr;
    virtual void* setOption(int, ...) = nullptr;
};

#define REGISTER_ZYGISK_MODULE(clazz) \
    static clazz __zygisk_module; \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api* api, JNIEnv* env) { \
        __zygisk_module.onLoad(api, env); \
    }

} // namespace zygisk

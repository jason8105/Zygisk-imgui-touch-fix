#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

class Api {
public:
    virtual void connectCompanion() = 0;
    virtual int getCompanionFd() = 0;
    virtual void setOption(uint32_t option) = 0;
    virtual void exemptFd(int fd) = 0;
};

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(Api *api, JNIEnv *env) {}
    virtual void postAppSpecialize(Api *api, JNIEnv *env) {}
    virtual void preServerSpecialize(Api *api, JNIEnv *env) {}
    virtual void postServerSpecialize(Api *api, JNIEnv *env) {}
};

enum Option : uint32_t {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_LIBRARY = 1,
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    module.onLoad(api, env); \
}

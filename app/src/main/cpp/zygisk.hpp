#pragma once

#include <jni.h>
#include <stdint.h>

namespace zygisk {

class Api;
class AppSpecimen;
class ServerSpecimen;

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecimen *specimen) {}
    virtual void postAppSpecialize(const AppSpecimen *specimen) {}
    virtual void preServerSpecialize(ServerSpecimen *specimen) {}
    virtual void postServerSpecialize(const ServerSpecimen *specimen) {}
};

enum Option : uint32_t {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_LIBRARY = 1,
};

class Api {
public:
    virtual int connectCompanion() = 0;
    virtual void setOption(Option option) = 0;
    virtual int getModuleDir() = 0;
    virtual void exemptFd(int fd) = 0;
};

class AppSpecimen {
public:
    virtual JNIEnv *getJNIEnv() const = 0;
    virtual const char *getNiceName() const = 0;
    virtual const char *getProcessName() const = 0;
};

class ServerSpecimen {
public:
    virtual JNIEnv *getJNIEnv() const = 0;
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    module.onLoad(api, env); \
}

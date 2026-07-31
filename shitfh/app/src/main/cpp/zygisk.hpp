#ifndef ZYGISK_HPP
#define ZYGISK_HPP

#include <jni.h>

namespace zygisk {

class Api;
class AppSpecializeArgs;
class ServerSpecializeArgs;

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
    virtual void connectCompanion(int socket) = 0;
    virtual void setOption(int option) = 0;
    virtual int getApiVersion() = 0;
};

enum Option {
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

#endif // ZYGISK_HPP

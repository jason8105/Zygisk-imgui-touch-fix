#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

namespace zygisk {

enum ApiVersion : uint32_t {
    V1 = 1,
};

enum Option : uint32_t {
    OPTION_DLCLOSE_MODULE_LIBRARY = 0,
};

class Api {
public:
    virtual void *jniGetEnv() = 0;
    virtual int option(Option opt) = 0;
    virtual void *module_load(const char *name, int flags) = 0;
    virtual void *plt_hook_register(const char *lib_name, const char *symbol, void *new_func, void **old_func) = 0;
    virtual bool plt_hook_commit() = 0;
    virtual int hook_jni_native_methods(const char *className, void *methods, int numMethods) = 0;
};

class ServerConnection {
public:
    virtual int connect() = 0;
    virtual void disconnect() = 0;
    virtual ssize_t send(const void *data, size_t length) = 0;
    virtual ssize_t recv(void *data, size_t length) = 0;
    virtual int send_fds(const int *fds, size_t length) = 0;
    virtual int recv_fds(int *fds, size_t length) = 0;
};

class ModuleBase {
public:
    virtual void onPreAppSpecialize(Api *api, ServerConnection *connection) {}
    virtual void onPostAppSpecialize(Api *api, ServerConnection *connection) {}
    virtual void onPreServerSpecialize(Api *api, ServerConnection *connection) {}
    virtual void onPostServerSpecialize(Api *api, ServerConnection *connection) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    static clazz __zygisk_module_instance; \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        /* compatibility bridge */ \
    } \
    extern "C" __attribute__((visibility("default"))) void zygisk_init(zygisk::Api *api, JNIEnv *env) { \
        __zygisk_module_instance.init(api, env); \
    }

#define ZYGISK_MODULE_WITH_ENV(clazz) \
    class clazz : public zygisk::ModuleBase

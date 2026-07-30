#pragma once

#include <jni.h>
#include <unistd.h>
#include <sys/types.h>

namespace zygisk {

class Api;
class Module;

enum Option {
    // SELinux context or namespace options if needed
    FORCE_DENYLIST_UNMOUNT = 1
};

class Module {
public:
    virtual ~Module() {}
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(Api *api, JNIEnv *env, const char *app_data_dir, const char *process_name) {}
    virtual void postAppSpecialize(Api *api, JNIEnv *env, const char *app_data_dir, const char *process_name) {}
    virtual void preServerSpecialize(Api *api, JNIEnv *env, const char *specialized_dir, const char *daemon) {}
    virtual void postServerSpecialize(Api *api, JNIEnv *env, const char *specialized_dir, const char *daemon) {}
};

class Api {
public:
    enum Flag {
        DLCLOSE_MODULE_LIBRARY = 1
    };

    virtual void *pltDlsym(const char *symbol) = 0;
    virtual void *hookPlt(const char *lib_name, const char *symbol, void *new_func, void **old_func) = 0;
    virtual void hookJniNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int numMethods) = 0;
    virtual int zapChildProcess() = 0;
    virtual void setOption(Option opt, bool enabled) = 0;
    virtual int getModuleFileDescriptor() = 0;
    virtual void *getSym(const char *symbol) = 0;
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    static clazz __zygisk_module_instance; \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        __zygisk_module_instance.onLoad(api, env); \
    }

#ifndef ZYGISK_HPP
#define ZYGISK_HPP

#include <jni.h>
#include <unistd.h>

namespace zygisk {

class ZygiskApi;
class ZygiskModule;

enum ApiFunction {
    kGetApiVersion = 0,
    kSetOption = 1,
    kHoodJniEnv = 2,
    kPassAppSpecializeArgs = 3,
    kPassServerSpecializeArgs = 4,
    kGetModuleDir = 5,
    kHookJniEnv = 2,
};

enum Option {
    kOptionHideMemory = 0,
    kOptionDlcloseModule = 1,
};

class Api {
public:
    virtual int GetApiVersion() = 0;
    virtual void SetOption(Option opt) = 0;
    virtual void *GetJNIEnv() = 0;
    virtual void *GetModuleDir() = 0;
    virtual void HookJniEnv(void *hook) = 0;
};

class Module {
public:
    virtual void OnLoad(Api *api, JNIEnv *env) {}
    virtual void PreAppSpecialize(void *args) {}
    virtual void PostAppSpecialize(void *args) {}
    virtual void PreServerSpecialize(void *args) {}
    virtual void PostServerSpecialize(void *args) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        static clazz module; \
        module.OnLoad(api, env); \
    }

#endif // ZYGISK_HPP

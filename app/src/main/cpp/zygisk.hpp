#pragma once
#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

class Api {
public:
    enum Option {
        OPTION_DLCLOSE_MODULE_LIBRARY = 0
    };
    virtual void *dlopen_api(const char *filename, int flag) = 0;
    virtual void *dlsym_api(const char *symbol) = 0;
    virtual void plt_hook_register(const char *lib_name, const char *symbol, void *fn, void **old_out) = 0;
    virtual bool plt_hook_commit() = 0;
    virtual void set_option(Option opt) = 0;
    virtual int get_module_version() = 0;
    virtual void *get_zj_iva() = 0;
};

class Module {
public:
    virtual void on_load(Api *api, JNIEnv *env) {}
    virtual void pre_app_specialize(void *args) {}
    virtual void post_app_specialize(void *args) {}
    virtual void pre_server_specialize(void *args) {}
    virtual void post_server_specialize(void *args) {}
    virtual ~Module() {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        static clazz module; \
        module.on_load(api, env); \
    }

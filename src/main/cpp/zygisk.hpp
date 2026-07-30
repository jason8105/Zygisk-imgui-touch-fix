#ifndef ZYGISK_HPP
#define ZYGISK_HPP

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

class Api {
public:
    enum Option {
        ZYGISK_OPTION_DLCLOSE_MODULE_LIBRARY = 0,
    };

    template <typename T>
    void pltHookRegister(const char *libname, const char *symbol, T new_func, T *old_func) {
        if (table && table->pltHookRegister) {
            table->pltHookRegister(this, libname, symbol, reinterpret_cast<void *>(new_func), reinterpret_cast<void **>(old_func));
        }
    }

    void *pltHookCommit() {
        if (table && table->pltHookCommit) {
            return table->pltHookCommit(this);
        }
        return nullptr;
    }

    int hookJniNativeMethods(JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
        if (table && table->hookJniNativeMethods) {
            return table->hookJniNativeMethods(this, env, className, methods, numMethods);
        }
        return JNI_FALSE;
    }

    void ezBloat(const char *placeholder, void *symbol) {
        if (table && table->ezBloat) {
            table->ezBloat(this, placeholder, symbol);
        }
    }

    void setOption(Option opt) {
        if (table && table->setOption) {
            table->setOption(this, opt);
        }
    }

    int getModuleInfo(char *path, size_t path_len, int *id) {
        if (table && table->getModuleInfo) {
            return table->getModuleInfo(this, path, path_len, id);
        }
        return 0;
    }

    void *xwrapDlopen(const char *filename, int flag) {
        if (table && table->xwrapDlopen) {
            return table->xwrapDlopen(this, filename, flag);
        }
        return nullptr;
    }

private:
    struct Table {
        void (*pltHookRegister)(Api *api, const char *libname, const char *symbol, void *new_func, void **old_func);
        void *(*pltHookCommit)(Api *api);
        int (*hookJniNativeMethods)(Api *api, JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods);
        void (*ezBloat)(Api *api, const char *placeholder, void *symbol);
        int (*setOption)(Api *api, Option opt);
        int (*getModuleInfo)(char *path, size_t path_len, int *id);
        void *(*xwrapDlopen)(Api *api, const char *filename, int flag);
    };

    Table *table;
};

class Module {
public:
    virtual ~Module() {}
    virtual void preAppSpecialize(void *cfg) {}
    virtual void postAppSpecialize(const void *args) {}
    virtual void preServerSpecialize(void *cfg) {}
    virtual void postServerSpecialize(const void *args) {}
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    static zygisk::Module *create_module() { return new clazz(); } \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        /* Magisk v24-26 entry point registration */ \
    }

#endif // ZYGISK_HPP

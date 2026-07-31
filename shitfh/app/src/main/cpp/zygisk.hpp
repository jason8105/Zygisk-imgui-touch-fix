#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

class Api;
class AppSpecimen;
class ServerSpecimen;

namespace Option {
enum : uint32_t {
    FORCE_DENYLIST_UNMOUNT = 1 << 0,
    DLCLOSE_MODULE_LIBRARY = 1 << 1,
};
}

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecimen *specimen) {}
    virtual void postAppSpecialize(const AppSpecimen *specimen) {}
    virtual void preServerSpecialize(ServerSpecimen *specimen) {}
    virtual void postServerSpecialize(const ServerSpecimen *specimen) {}
};

class AppSpecimen {
public:
    const char *package_name;
    const char *process_name;
    const char *app_data_dir;
};

class ServerSpecimen {
public:
    const char *process_name;
};

struct ApiTable {
    int (*connectCompanion)(void *);
    void (*setOption)(void *, uint32_t);
    void (*exemptAppEnv)(void *);
};

class Api {
public:
    void *impl;
    ApiTable *tbl;

    int connectCompanion() {
        return tbl ? tbl->connectCompanion(impl) : -1;
    }
    void setOption(Option::enum_t option) {
        if (tbl) tbl->setOption(impl, static_cast<uint32_t>(option));
    }
    void exemptAppEnv() {
        if (tbl) tbl->exemptAppEnv(impl);
    }
};

struct ModuleAbi {
    long api_version;
    ModuleBase *impl;
    void (*preAppSpecialize)(ModuleBase *, AppSpecimen *);
    void (*postAppSpecialize)(ModuleBase *, const AppSpecimen *);
    void (*preServerSpecialize)(ModuleBase *, ServerSpecimen *);
    void (*postServerSpecialize)(ModuleBase *, const ServerSpecimen *);
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
static void _preAppSpecialize(zygisk::ModuleBase *m, zygisk::AppSpecimen *s) { \
    m->preAppSpecialize(s); \
} \
static void _postAppSpecialize(zygisk::ModuleBase *m, const zygisk::AppSpecimen *s) { \
    m->postAppSpecialize(s); \
} \
static void _preServerSpecialize(zygisk::ModuleBase *m, zygisk::ServerSpecimen *s) { \
    m->preServerSpecialize(s); \
} \
static void _postServerSpecialize(zygisk::ModuleBase *m, const zygisk::ServerSpecimen *s) { \
    m->postServerSpecialize(s); \
} \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    module.onLoad(api, env); \
    static zygisk::ModuleAbi abi{ \
        .api_version = 1, \
        .impl = &module, \
        .preAppSpecialize = _preAppSpecialize, \
        .postAppSpecialize = _postAppSpecialize, \
        .preServerSpecialize = _preServerSpecialize, \
        .postServerSpecialize = _postServerSpecialize, \
    }; \
    *(zygisk::ModuleAbi **)api = &abi; \
}

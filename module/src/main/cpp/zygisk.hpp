#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

struct ApiTable;
class AppSpecializeArgs;
class ServerSpecializeArgs;

enum Option : uint32_t {
    FORCE_DENYLIST_UNMOUNT = 1 << 0,
    DLCLOSE_MODULE_LIBRARY = 1 << 1,
};

class ModuleBase {
public:
    virtual void onLoad(ApiTable *table, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

struct ApiTable {
    void *impl;
    bool (*registerModule)(ApiTable *, ModuleBase *);
    void (*setOption)(ApiTable *, Option);
    int (*getModuleDir)(ApiTable *);
    bool (*exemptAppProcess)(ApiTable *);
    int (*connectCompanion)(ApiTable *);
};

class Api {
    ApiTable *tbl;
public:
    Api(ApiTable *t) : tbl(t) {}
    bool registerModule(ModuleBase *module) {
        return tbl && tbl->registerModule ? tbl->registerModule(tbl, module) : false;
    }
    void setOption(Option opt) {
        if (tbl && tbl->setOption) tbl->setOption(tbl, opt);
    }
    int getModuleDir() {
        return (tbl && tbl->getModuleDir) ? tbl->getModuleDir(tbl) : -1;
    }
    bool exemptAppProcess() {
        return (tbl && tbl->exemptAppProcess) ? tbl->exemptAppProcess(tbl) : false;
    }
    int connectCompanion() {
        return (tbl && tbl->connectCompanion) ? tbl->connectCompanion(tbl) : -1;
    }
};

class AppSpecializeArgs {
public:
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *rlimits;
    jint *mount_external;
    jstring *seinfo;
    jstring *nice_name;
    jintArray *is_child_zygote;
    jstring *instruction_set;
    jstring *app_data_dir;
    jboolean *is_top_app;
    jobjectArray *pkg_data_info_list;
    jobjectArray *whitelisted_data_info_list;
    jboolean *mount_data_dirs;
    jboolean *mount_storage_dirs;
};

class ServerSpecializeArgs {
public:
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *rlimits;
    jint *permitted_capabilities;
    jint *effective_capabilities;
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(zygisk::ApiTable *table, JNIEnv *env) { \
    zygisk::Api api{table}; \
    static clazz module; \
    api.registerModule(&module); \
}

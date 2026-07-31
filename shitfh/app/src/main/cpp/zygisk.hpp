#ifndef ZYGISK_HPP_
#define ZYGISK_HPP_

#include <jni.h>
#include <stdint.h>

namespace zygisk {

enum Option : uint32_t {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_LIBRARY = 1,
};

struct AppSpecializeArgs {
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

struct ServerSpecializeArgs {
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

class Api;

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

struct ApiTable {
    ModuleBase *impl;
    bool (*registerModule)(ApiTable *, ModuleBase *);
    bool (*connectCompanion)(ApiTable *);
    int (*getCompanionSocket)(ApiTable *);
    void (*setOption)(ApiTable *, Option);
    void (*exemptAppProcess)(ApiTable *);
};

class Api {
    ApiTable *tbl;
public:
    Api(ApiTable *t) : tbl(t) {}
    bool connectCompanion() { return tbl->connectCompanion(tbl); }
    int getCompanionSocket() { return tbl->getCompanionSocket(tbl); }
    void setOption(Option opt) { tbl->setOption(tbl, opt); }
    void exemptAppProcess() { tbl->exemptAppProcess(tbl); }
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
static bool zygisk_module_init(zygisk::ApiTable *table) { \
    static clazz module; \
    return table->registerModule(table, &module); \
} \
extern "C" __attribute__((visibility("default"))) \
void zygisk_module_entry(zygisk::ApiTable *table) { \
    table->registerModule(table, new clazz()); \
}

#endif // ZYGISK_HPP_

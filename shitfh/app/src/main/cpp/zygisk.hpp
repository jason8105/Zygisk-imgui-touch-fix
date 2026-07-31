#pragma once

#include <jni.h>
#include <stdint.h>

namespace zygisk {

enum Option : int {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_LIBRARY = 1,
};

struct AppSpecializeArgs {
    jint* uid;
    jint* gid;
    jintArray* gids;
    jint* runtime_flags;
    jobjectArray* rlimits;
    jint* mount_external;
    jstring* seinfo;
    jstring* nice_name;
    jintArray* is_child_zygote;
    jstring* instruction_set;
    jstring* app_data_dir;
};

struct ServerSpecializeArgs {
    jint* uid;
    jint* gid;
    jintArray* gids;
    jint* runtime_flags;
    jlong* permitted_capabilities;
    jlong* effective_capabilities;
};

class Api;

class ModuleBase {
public:
    virtual void onLoad(Api* api, JNIEnv* env) {}
    virtual void preAppSpecialize(AppSpecializeArgs* args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs* args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs* args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs* args) {}
};

struct Api {
    bool (*connectCompanion)(Api* api);
    int (*getCompanionSocket)(Api* api);
    void (*setOption)(Api* api, Option opt);
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" __attribute__((visibility("default"))) \
void zygisk_module_entry(zygisk::Api* api, JNIEnv* env) { \
    static clazz instance; \
    instance.onLoad(api, env); \
}

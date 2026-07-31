#pragma once

#include <jni.h>
#include <stdint.h>

namespace zygisk {

struct AppSpecializeArgs {
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *rlimits;
    jint *mount_external;
    jstring *se_info;
    jstring *nice_name;
    jintArray *is_child_zygote;
    jstring *instruction_set;
    jstring *app_data_dir;
};

struct ServerSpecializeArgs {
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *rlimits;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

class OptionSet;
class Api;

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
    virtual bool registerModule(ModuleBase *module) = 0;
    virtual int getModuleFlags() = 0;
    virtual void setOption(OptionSet opt) = 0;
    virtual int getApiVersion() = 0;
    virtual int connectCompanion() = 0;
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
static void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    static clazz module; \
    api->registerModule(&module); \
    module.onLoad(api, env); \
} \
extern "C" __attribute__((visibility("default"))) \
void zygisk_module_entry_v1(zygisk::Api *api, JNIEnv *env) { \
    zygisk_module_entry(api, env); \
}

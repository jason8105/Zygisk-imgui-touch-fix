#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

class Api;
class AppSpec;
class ServerSpec;

enum Option : uint32_t {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_LIBRARY = 1,
};

class ModuleBase {
public:
    virtual void onLoad(Api *api, JNIEnv *env) {}
    virtual void preAppSpec(AppSpec *spec) {}
    virtual void postAppSpec(AppSpec *spec) {}
    virtual void preServerSpec(ServerSpec *spec) {}
    virtual void postServerSpec(ServerSpec *spec) {}
};

class Api {
public:
    virtual int connectCompanion() = 0;
    virtual void setOption(Option opt) = 0;
    virtual bool exemptFd(int fd) = 0;
};

class AppSpec {
public:
    const char *nice_name;
    const char *app_data_dir;
    int uid;
    int gid;
    int *gids;
    int gids_count;
    int mount_external;
    const char *seinfo;
    const char *nice_name_ptr;
    bool is_child_zygote;
};

class ServerSpec {
public:
    int uid;
    int gid;
    int *gids;
    int gids_count;
};

} // namespace zygisk

extern "C" {
    void zygisk_module_entry(zygisk::Api *api, JNIEnv *env);
}

#define REGISTER_ZYGISK_MODULE(className) \
static className *g_zygisk_module_instance = nullptr; \
extern "C" [[gnu::visibility("default")]] void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
    g_zygisk_module_instance = new className(); \
    g_zygisk_module_instance->onLoad(api, env); \
}

#pragma once

#include <stdint.h>
#include <stddef.h>

namespace zygisk {

enum ApiVersion : int {
    kApiVersion1 = 1,
    kApiVersion2 = 2,
    kApiVersion3 = 3,
    kApiVersion4 = 4,
    kApiVersion5 = 5,
};

enum Reason : int {
    kReasonUnknown = 0,
    kReasonNoMatch = 1,
    kReasonDeadCode = 2,
};

struct LoaderAPI;

class AppSpecializeArgs {
public:
    int uid;
    int gid;
    const char *gids;
    int runtime_flags;
    int mount_external;
    const char *seinfo;
    const char *nice_name;
    const char *instruction_set;
    const char *app_data_dir;
};

class ServerSpecializeArgs {
public:
    int uid;
    int gid;
    const char *gids;
    int runtime_flags;
    const char *set_args;
};

class ZygiskModule {
public:
    virtual ~ZygiskModule() {}
    virtual void onPreSpecialize(AppSpecializeArgs *args) {}
    virtual void onPostSpecialize(AppSpecializeArgs *args) {}
    virtual void onPreSpecialize(ServerSpecializeArgs *args) {}
    virtual void onPostSpecialize(ServerSpecializeArgs *args) {}
};

struct LoaderAPI {
    ApiVersion api_version;
    void *internal;
    
    bool (*registerModule)(LoaderAPI *api, ZygiskModule *module);
    void *(*holdOpenJDK)(LoaderAPI *api);
    void *(*plt_hook_register)(LoaderAPI *api, const char *libpath, const char *symbol, void *new_func, void **old_func);
    void *(*plt_hook_commit)(LoaderAPI *api);
    void *(*exemptFd)(LoaderAPI *api, int fd);
    void *(*conntectCompanion)(LoaderAPI *api);
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
    static class clazz##_init { \
    public: \
        clazz##_init() { \
            auto module = new clazz(); \
            // Registration handled in entrypoint \
        } \
    } clazz##_init_instance;

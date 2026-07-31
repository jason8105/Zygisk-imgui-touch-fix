/* SPDX-License-Identifier: MIT */
/* Copyright 2021-2023 John Wu */

#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

#define ZYGISK_API_VERSION 4

namespace zygisk {

struct ApiTable;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;

class ModuleBase {
public:
    virtual ~ModuleBase() = default;
    virtual void onLoad(ApiTable *api, jclass clazz) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

struct AppSpecializeArgs {
    jstring &process_name;
    jstring &package_name;
    jobjectArray &app_data_dirs;
    jint &uid;
    jint &gid;
    jarray &gids;
    jint &runtime_flags;
    jobjectArray &is_child_zygote;
    jstring &instruction_set;
    jstring &app_data_dir;
    jboolean &is_top_app;
    jobjectArray &pkg_data_info_list;
    jobjectArray &whitelisted_data_info_list;
    jboolean &mount_external_storage;
};

struct ServerSpecializeArgs {
    jint &uid;
    jint &gid;
    jarray &gids;
    jint &runtime_flags;
    jlong &permitted_capabilities;
    jlong &effective_capabilities;
};

struct ApiTable {
    int32_t v; // API version
    void (*registerModule)(ApiTable *, ModuleBase *);
    void (*setOption)(ApiTable *, int);
    int (*getEventFd)(ApiTable *);
    const char *(*getArgString)(ApiTable *, jstring);
};

enum Option {
    FORCE_DENYLIST = 0,
    DLCLOSE_MODULE_LIBRARY = 1
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" void zygisk_module_entry(zygisk::ApiTable *api, JNIEnv *env) { \
    static clazz module; \
    api->registerModule(api, &module); \
}

/* Copyright 2021-2023 John Wu
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <jni.h>
#include <stddef.h>
#include <stdint.h>

namespace zygisk {

struct ApiTable;
class Api;

struct AppSpecializeArgs {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jobjectArray &rlimits;
    jint &mount_external;
    jstring &se_info;
    jstring &nice_name;
    jboolean &is_child_zygote;
    jstring &instruction_set;
    jstring &app_data_dir;
    jboolean &is_top_app;
    jobjectArray &pkg_data_info_list;
    jobjectArray &whitelisted_data_info_list;
    jboolean &mount_data_dirs;
    jboolean &mount_storage_dirs;

    AppSpecializeArgs(
        jint &uid, jint &gid, jintArray &gids, jint &runtime_flags, jobjectArray &rlimits,
        jint &mount_external, jstring &se_info, jstring &nice_name, jboolean &is_child_zygote,
        jstring &instruction_set, jstring &app_data_dir, jboolean &is_top_app,
        jobjectArray &pkg_data_info_list, jobjectArray &whitelisted_data_info_list,
        jboolean &mount_data_dirs, jboolean &mount_storage_dirs)
        : uid(uid), gid(gid), gids(gids), runtime_flags(runtime_flags), rlimits(rlimits),
          mount_external(mount_external), se_info(se_info), nice_name(nice_name),
          is_child_zygote(is_child_zygote), instruction_set(instruction_set),
          app_data_dir(app_data_dir), is_top_app(is_top_app),
          pkg_data_info_list(pkg_data_info_list),
          whitelisted_data_info_list(whitelisted_data_info_list),
          mount_data_dirs(mount_data_dirs), mount_storage_dirs(mount_storage_dirs) {}
};

struct ServerSpecializeArgs {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jobjectArray &rlimits;
    jlong &permitted_capabilities;
    jlong &effective_capabilities;

    ServerSpecializeArgs(
        jint &uid, jint &gid, jintArray &gids, jint &runtime_flags, jobjectArray &rlimits,
        jlong &permitted_capabilities, jlong &effective_capabilities)
        : uid(uid), gid(gid), gids(gids), runtime_flags(runtime_flags), rlimits(rlimits),
          permitted_capabilities(permitted_capabilities),
          effective_capabilities(effective_capabilities) {}
};

class ModuleBase {
public:
    virtual ~ModuleBase() {}
    virtual void onLoad(Api *api, jclass clazz) {}
    virtual void preAppSpecialize(AppSpecializeArgs *args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
};

enum StateFlags : uint32_t {
    PROCESS_ON_APP_COMPANION = 1U << 0,
    PROCESS_GRANTED_ROOT = 1U << 1,
};

class Api {
public:
    bool connectCompanion() {
        return impl->connectCompanion(this);
    }
    int getCompanionFd() {
        return impl->getCompanionFd(this);
    }
    int getArgc() {
        return impl->getArgc(this);
    }
    char **getArgv() {
        return impl->getArgv(this);
    }
    void setOption(uint32_t option) {
        impl->setOption(this, option);
    }
    uint32_t getFlags() {
        return impl->getFlags(this);
    }
    const char *getArgString(jstring str) {
        return impl->getArgString(this, str);
    }
    void releaseArgString(jstring str, const char *val) {
        impl->releaseArgString(this, str, val);
    }
private:
    const ApiTable *impl;
    friend class ModuleBase;
};

struct ApiTable {
    bool (*connectCompanion)(Api *);
    int (*getCompanionFd)(Api *);
    int (*getArgc)(Api *);
    char **(*getArgv)(Api *);
    void (*setOption)(Api *, uint32_t);
    uint32_t (*getFlags)(Api *);
    const char *(*getArgString)(Api *, jstring);
    void (*releaseArgString)(Api *, jstring, const char *);
};

using RegisterModule_t = void (*)(const ApiTable *, ModuleBase *);

} // namespace zygisk

extern "C" void zygisk_module_entry(const zygisk::ApiTable *, zygisk::RegisterModule_t);

#define REGISTER_ZYGISK_MODULE(clazz) \
extern "C" [[gnu::visibility("default")]] \
void zygisk_module_entry(const zygisk::ApiTable *table, zygisk::RegisterModule_t register_module) { \
    static clazz module; \
    register_module(table, &module); \
}

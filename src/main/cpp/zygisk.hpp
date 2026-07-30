#ifndef ZYGISK_HPP
#define ZYGISK_HPP

#include <jni.h>

namespace zygisk {

enum ApiVersion {
    API_VERSION_1 = 1,
    API_VERSION_2 = 2,
    API_VERSION_3 = 3,
    API_VERSION_4 = 4,
    API_VERSION_5 = 5,
    API_VERSION_CURRENT = 5
};

struct AppSpecializeArgs {
    jint uid;
    jint gid;
    jobjectArray gids;
    jint runtimeFlags;
    jobjectArray rlimit;
    jint mountExternal;
    jstring seInfo;
    jstring niceName;
    jstring instructionSet;
    jstring appDataDir;
};

struct ServerSpecializeArgs {
    jint uid;
    jint gid;
    jobjectArray gids;
    jint runtimeFlags;
    jobjectArray rlimit;
    jstring seInfo;
    jstring name;
};

class Api {
public:
    virtual void registerModule(void* module) = 0;
    virtual int diockServer(int req) = 0;
    virtual void setOption(int option) = 0;
    virtual int connectCompanion() = 0;
};

class ModuleBase {
public:
    virtual ~ModuleBase() {}
    virtual void onLoad(Api* api, JNIEnv* env) {}
    virtual void preAppSpecialize(AppSpecializeArgs* args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs* args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs* args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs* args) {}
};

struct ModuleABI {
    int apiVersion;
    void (*companion)(int);
    void (*moduleEntry)(Api*, JNIEnv*);
};

} // namespace zygisk

#endif // ZYGISK_HPP

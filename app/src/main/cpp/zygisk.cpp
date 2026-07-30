#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include "zygisk.hpp"
#include "imgui.h"

#define LOG_TAG "ZygiskImGui"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

void InitializeInputHooks();

class ZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Disable or inject before app specialization if required
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        LOGD("ZygiskImGui loaded into target process.");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        InitializeInputHooks();
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
};

static zygisk::Api *g_api = nullptr;

REGISTER_ZYGISK_MODULE(ZygiskModule)

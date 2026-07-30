#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include "zygisk.hpp"
#include "imgui.h"

#define LOG_TAG "ZygiskModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

void init_input_hooks();

class ImGuiZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        // Prepare environment prior to app specialization
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        LOGD("Zygisk module loaded into target application process.");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        init_input_hooks();
    }

private:
    zygisk::Api* api = nullptr;
    JNIEnv* env = nullptr;
};

REGISTER_ZYGISK_MODULE(ImGuiZygiskModule)

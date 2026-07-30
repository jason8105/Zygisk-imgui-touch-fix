#include <jni.h>
#include <unistd.h>
#include <android/log.h>
#include "zygisk.hpp"
#include "imgui.h"

#define LOG_TAG "ZygiskImgui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void InitInputHooks();

class ImGuiZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        // Executed before app specialization
    }

    void postAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        LOGI("ImGui Zygisk Module loaded into target process.");
        
        // Initialize ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        // Initialize universal touch hook
        InitInputHooks();
    }

private:
    zygisk::Api* api = nullptr;
    JNIEnv* env = nullptr;
};

// Register module for Magisk v24-26 Zygisk API
REGISTER_ZYGISK_MODULE(ImGuiZygiskModule)

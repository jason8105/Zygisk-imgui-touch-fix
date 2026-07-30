#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>
#include <pthread.h>
#include "zygisk.hpp"
#include "imgui/imgui.h"

#define TAG "ZygiskImguiTouchFix"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Universal Touch Hook & ImGui Integration
typedef int (*AInputQueue_getEvent_t)(AInputQueue* queue, AInputEvent** outEvent);
typedef int32_t (*AInputEvent_getType_t)(const AInputEvent* event);
typedef int32_t (*AMotionEvent_getAction_t)(const AInputEvent* event);
typedef float (*AMotionEvent_getX_t)(const AInputEvent* event, size_t pointer_index);
typedef float (*AMotionEvent_getY_t)(const AInputEvent* event, size_t pointer_index);

static AInputQueue_getEvent_t orig_AInputQueue_getEvent = nullptr;
static AInputEvent_getType_t orig_AInputEvent_getType = nullptr;
static AMotionEvent_getAction_t orig_AMotionEvent_getAction = nullptr;
static AMotionEvent_getX_t orig_AMotionEvent_getX = nullptr;
static AMotionEvent_getY_t orig_AMotionEvent_getY = nullptr;

static bool g_menuVisible = true;

int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int ret = orig_AInputQueue_getEvent(queue, outEvent);
    if (ret == 1 && outEvent && *outEvent) {
        if (orig_AInputEvent_getType && orig_AInputEvent_getType(*outEvent) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t actionMasked = orig_AMotionEvent_getAction ? (orig_AMotionEvent_getAction(*outEvent) & AMOTION_EVENT_ACTION_MASK) : 0;
            float x = orig_AMotionEvent_getX ? orig_AMotionEvent_getX(*outEvent, 0) : 0.0f;
            float y = orig_AMotionEvent_getY ? orig_AMotionEvent_getY(*outEvent, 0) : 0.0f;

            auto& io = ImGui::GetIO();
            io.AddMousePosEvent(x, y);

            if (actionMasked == AMOTION_EVENT_ACTION_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
                io.AddMouseButtonEvent(0, false);
            }
        }
    }
    return ret;
}

class ZygiskImguiModule : public zygisk::ModuleBase {
public:
    void init(zygisk::Api *api, JNIEnv *env) {
        this->api = api;
        this->env = env;
    }

    void onPostAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        LOGI("onPostAppSpecialize invoked - hooking input events universally");
        
        // Resolve symbols for universal AInputQueue and AMotionEvent hooking
        void* libandroid = dlopen("libandroid.so", RTLD_GLOBAL | RTLD_LAZY);
        if (libandroid) {
            orig_AInputQueue_getEvent = (AInputQueue_getEvent_t)dlsym(libandroid, "AInputQueue_getEvent");
            orig_AInputEvent_getType = (AInputEvent_getType_t)dlsym(libandroid, "AInputEvent_getType");
            orig_AMotionEvent_getAction = (AMotionEvent_getAction_t)dlsym(libandroid, "AMotionEvent_getAction");
            orig_AMotionEvent_getX = (AMotionEvent_getX_t)dlsym(libandroid, "AMotionEvent_getX");
            orig_AMotionEvent_getY = (AMotionEvent_getY_t)dlsym(libandroid, "AMotionEvent_getY");

            if (orig_AInputQueue_getEvent) {
                api->pltHookRegister("libandroid.so", "AInputQueue_getEvent", (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
            }
        }

        ImGui::CreateContext();
        LOGI("ImGui context initialized successfully with universal touch fix.");
    }

private:
    zygisk::Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(ZygiskImguiModule)

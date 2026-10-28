#pragma once
#include <jni.h>
#include <android/log.h>
#include <sys/types.h>
#include <string.h>
#include <xhook.h>
#include "zygisk.hpp"

#ifndef LOG
#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "PubgCheat", __VA_ARGS__)
#endif

void SetupHooks();

class XHookModule : public zygisk::ModuleBase {
public:
    zygisk::Api *api;
    JNIEnv *env;

    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        LOG(">>> Zygisk Module onLoad called! <<<");
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        const char* process = env->GetStringUTFChars(args->nice_name, nullptr);
        LOG("preAppSpecialize for process: %s", process);
        
        // Проверяем Global или Taiwan пакет
        if (strstr(process, "com.tencent.ig") || strstr(process, "com.rekoo.pubgm")) {
            LOG("Target game found! Continuing injection.");
        } else {
            LOG("Not target game, unloading module.");
            api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        LOG(">>> postAppSpecialize called! Setting up hooks... <<<");
        SetupHooks();
    }
};

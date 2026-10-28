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

// Объявляем функцию, которая будет в module.cpp
void SetupHooks();

class XHookModule : public zygisk::ModuleBase {
public:
    zygisk::Api *api;
    JNIEnv *env;

    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        // Проверяем, что это игра (Глобалка или Тайвань)
        const char* process = env->GetStringUTFChars(args->nice_name, nullptr);
        if (strstr(process, "com.tencent.ig") || strstr(process, "com.rekoo.pubgm")) {
            // Игра найдена, ничего не делаем, пропускаем
        } else {
            // Если это не игра, выгружаем модуль
            api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        // Здесь вызываем функцию из module.cpp
        SetupHooks();
    }
};

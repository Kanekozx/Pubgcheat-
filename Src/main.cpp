#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#define LOG_TAG "Internal"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Подключаем ImGui и хуки (пока в виде заглушек)
// #include "imgui.h"
// #include "dobby.h"

bool g_Initialized = false;
uintptr_t g_UE4Base = 0;

// Оригинальная функция отрисовки
EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

// Наша подмененная функция
EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    // Здесь мы будем рисовать наше меню и ESP
    if (!g_Initialized) {
        LOGD("EgL Hooked! Initializing ImGui...");
        // InitImGui(...)
        g_Initialized = true;
    }

    // Вызов оригинальной функции, чтобы игра рендерилась
    return orig_eglSwapBuffers(dpy, surface);
}

// Точка входа при загрузке библиотеки
__attribute__((constructor))
void main_init() {
    LOGD("Internal Cheat Loaded!");

    // Ждем загрузки библиотеки игры
    void* handle = dlopen("libUE4.so", RTLD_NOW);
    if (handle) {
        g_UE4Base = (uintptr_t)dlsym(handle, "JNI_OnLoad"); // Примерно так ищется база
        LOGD("libUE4.so found at: %p", (void*)g_UE4Base);
        
        // Находим eglSwapBuffers и ставим хук через Dobby
        void* eglSwapBuffers = dlsym(RTLD_NEXT, "eglSwapBuffers");
        if (eglSwapBuffers) {
            // DobbyHook(eglSwapBuffers, (void*)hooked_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGD("Hook installed on eglSwapBuffers");
        }
    }
}

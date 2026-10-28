#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <string.h>
#include "xhook_api.h"

// Объявляем функции из menu.cpp, чтобы компилятор не ругался
void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h);
void DrawESP();
void DrawMenu();
void ImGuiRender();

// Указатель на оригинальную функцию eglSwapBuffers
using eglSwapBuffers_t = EGLBoolean(*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t origSwap = nullptr;

// Наш хук на отрисовку
static EGLBoolean hookSwap(EGLDisplay dpy, EGLSurface surf) {
    static bool inited = false;
    
    // Инициализируем ImGui при первом кадре
    if (!inited) {
        EGLint _w = 0, _h = 0;
        eglQuerySurface(dpy, surf, EGL_WIDTH, &_w);
        eglQuerySurface(dpy, surf, EGL_HEIGHT, &_h);
        
        ImGuiInit(dpy, surf, _w, _h);
        inited = true;
    }

    // Рисуем наш чит поверх игры
    DrawESP();
    DrawMenu();
    ImGuiRender();

    // Вызываем оригинальную функцию, чтобы игра работала дальше
    return origSwap(dpy, surf);
}

// Функция, которую вызывает xhook_api.h
void SetupHooks() {
    LOG("Setting up hooks...");
    
    // Регистрируем хук на eglSwapBuffers в libEGL.so
    int r = xhook_register(".*libEGL.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    if (r != 0) {
        LOG("xhook_register failed: %d", r);
    }
    
    // Применяем хуки
    r = xhook_refresh(0);
    if (r != 0) {
        LOG("xhook_refresh failed: %d", r);
    }
}

// Регистрируем наш Zygisk модуль
REGISTER_ZYGISK_MODULE(XHookModule)

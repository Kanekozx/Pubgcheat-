#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <xhook.h>

#ifndef LOG
#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "PubgCheat", __VA_ARGS__)
#endif

// Объявляем функции из menu.cpp
void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h);
void DrawESP();
void DrawMenu();
void ImGuiRender();

using eglSwapBuffers_t = EGLBoolean(*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t origSwap = nullptr;

static EGLBoolean hookSwap(EGLDisplay dpy, EGLSurface surf) {
    static bool inited = false;
    
    if (!inited) {
        EGLint _w = 0, _h = 0;
        eglQuerySurface(dpy, surf, EGL_WIDTH, &_w);
        eglQuerySurface(dpy, surf, EGL_HEIGHT, &_h);
        
        if (_w > 0 && _h > 0) {
            LOG("Screen ready! W: %d, H: %d", _w, _h);
            ImGuiInit(dpy, surf, _w, _h);
            inited = true;
        } else {
            return origSwap(dpy, surf);
        }
    }

    DrawESP();
    DrawMenu();
    ImGuiRender();

    return origSwap(dpy, surf);
}

// Поток для установки хуков
void* SetupThread(void*) {
    sleep(3); // Ждем, пока игра чуть-чуть прогрузится
    LOG("Setting up hooks via injector...");
    
    int r = xhook_register(".*libEGL.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    if (r == 0) {
        LOG("xhook_register success!");
    } else {
        LOG("xhook_register failed: %d", r);
    }
    
    xhook_refresh(0);
    return nullptr;
}

// ВОТ ЭТА СТРОКА ЗАСТАВИТ БИБЛИОТЕКУ ЗАПУСТИТЬСЯ СРАЗУ ПОСЛЕ ИНЖЕКТА
__attribute__((constructor))
void onLibraryLoaded() {
    LOG(">>> Library injected via MT Manager/Termux! <<<");
    pthread_t thread;
    pthread_create(&thread, nullptr, SetupThread, nullptr);
}

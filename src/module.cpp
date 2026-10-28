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

#define LOG_TAG "PubgCheat"
#define LOG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Объявления функций из menu.cpp
void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h);
void DrawESP();
void DrawMenu();
void ImGuiRender();

using eglSwapBuffers_t = EGLBoolean(*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t origSwap = nullptr;

// Наш хук на отрисовку
static EGLBoolean hookSwap(EGLDisplay dpy, EGLSurface surf) {
    static bool inited = false;
    
    if (!inited) {
        EGLint _w = 0, _h = 0;
        eglQuerySurface(dpy, surf, EGL_WIDTH, &_w);
        eglQuerySurface(dpy, surf, EGL_HEIGHT, &_h);
        
        // Ждем, пока экран прогрузится
        if (_w > 0 && _h > 0) {
            LOG("Screen ready! W: %d, H: %d", _w, _h);
            ImGuiInit(dpy, surf, _w, _h);
            inited = true;
        } else {
            return origSwap(dpy, surf);
        }
    }

    // Рисуем меню и ESP
    DrawESP();
    DrawMenu();
    ImGuiRender();

    return origSwap(dpy, surf);
}

// Поток для установки хуков
void* SetupThread(void*) {
    LOG("Waiting for libUE4.so to load...");
    
    // Ждем, пока игра загрузит движок libUE4.so (до 20 секунд)
    int retries = 0;
    while (retries < 20) {
        void* handle = dlopen("libUE4.so", RTLD_NOLOAD);
        if (handle) {
            dlclose(handle);
            break; // Движок загружен, выходим из цикла
        }
        sleep(1);
        retries++;
    }
    
    sleep(2); // Даем движку пару секунд, чтобы он начал рисовать
    LOG(">>> Setting up hooks via injector... <<<");
    
    // ХУКАЕМ ВЫЗОВЫ ИЗ libUE4.so (движка игры)
    int r1 = xhook_register(".*libUE4.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    LOG("eglSwapBuffers hook: %d", r1);

    // На случай, если движок использует расширенную версию функции
    void* origSwapDamage = nullptr;
    int r2 = xhook_register(".*libUE4.so$", "eglSwapBuffersWithDamageKHR", (void*)hookSwap, (void**)&origSwapDamage);
    LOG("eglSwapBuffersWithDamageKHR hook: %d", r2);
    
    if (r1 == 0 || r2 == 0) {
        xhook_refresh(0);
        LOG("xhook_refresh success!");
    } else {
        LOG("xhook_register FAILED!");
    }
    
    return nullptr;
}
// Экспортируемая функция для инжектора
extern "C" int main() { return 0; }

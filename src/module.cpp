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
#include <stdio.h>

// Логи пишем в файл, так как KernelSU прячет logcat
void log_to_file(const char* msg) {
    FILE* f = fopen("/data/local/tmp/cheat_log.txt", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

// Объявляем функции из menu.cpp
void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h);
void ImGuiBeginFrame();
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
            char buf[128];
            sprintf(buf, "Screen ready! W: %d, H: %d", _w, _h);
            log_to_file(buf);
            ImGuiInit(dpy, surf, _w, _h);
            inited = true;
        } else {
            return origSwap(dpy, surf);
        }
    }

    // Важно: NewFrame должен быть здесь!
    ImGuiBeginFrame();
    DrawESP();
    DrawMenu();
    ImGuiRender();

    return origSwap(dpy, surf);
}

void* SetupThread(void*) {
    log_to_file(">>> Thread started! Waiting for libUE4.so... <<<");
    
    int retries = 0;
    while (retries < 20) {
        void* handle = dlopen("libUE4.so", RTLD_NOLOAD);
        if (handle) {
            dlclose(handle);
            break; 
        }
        sleep(1);
        retries++;
    }
    
    sleep(2); 
    log_to_file(">>> Setting up hooks... <<<");
    
    int r1 = xhook_register(".*libUE4.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    char buf[128];
    sprintf(buf, "eglSwapBuffers hook: %d", r1);
    log_to_file(buf);

    void* origSwapDamage = nullptr;
    int r2 = xhook_register(".*libUE4.so$", "eglSwapBuffersWithDamageKHR", (void*)hookSwap, (void**)&origSwapDamage);
    sprintf(buf, "eglSwapBuffersWithDamageKHR hook: %d", r2);
    log_to_file(buf);
    
    if (r1 == 0 || r2 == 0) {
        xhook_refresh(0);
        log_to_file("xhook_refresh success!");
    } else {
        log_to_file("xhook_register FAILED!");
    }
    
    return nullptr;
}

__attribute__((constructor))
void onLibraryLoaded() {
    log_to_file(">>> Library injected! <<<");
    pthread_t thread;
    pthread_create(&thread, nullptr, SetupThread, nullptr);
}

extern "C" int main() { return 0; }

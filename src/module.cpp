#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <pthread.h>
#include <unistd.h>
#include <xhook.h>
#include "imgui.h"

#define LOG_TAG "UE4"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h);
void ImGuiBeginFrame();
void DrawESP();
void DrawMenu();
void ImGuiRender();

using eglSwapBuffers_t = EGLBoolean(*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t origSwap = nullptr;

using eglSwapBuffersWithDamageKHR_t = EGLBoolean(*)(EGLDisplay, EGLSurface, EGLint*, EGLint);
static eglSwapBuffersWithDamageKHR_t origSwapDamage = nullptr;

void DrawFrame(EGLDisplay dpy, EGLSurface surf) {
    static bool inited = false;
    if (!inited) {
        EGLint _w = 0, _h = 0;
        eglQuerySurface(dpy, surf, EGL_WIDTH, &_w);
        eglQuerySurface(dpy, surf, EGL_HEIGHT, &_h);
        if (_w > 0 && _h > 0) {
            LOGI("ImGui Initialized: %dx%d", _w, _h);
            ImGuiInit(dpy, surf, _w, _h);
            inited = true;
        }
    }
    
    if (inited) {
        ImGuiBeginFrame();
        DrawESP(); // Рисуем ESP
        ImGuiRender();
    }
}

static EGLBoolean hookSwap(EGLDisplay dpy, EGLSurface surf) {
    DrawFrame(dpy, surf);
    return origSwap(dpy, surf);
}

static EGLBoolean hookSwapDamage(EGLDisplay dpy, EGLSurface surf, EGLint* rects, EGLint n_rects) {
    DrawFrame(dpy, surf);
    return origSwapDamage(dpy, surf, rects, n_rects);
}

void* SetupThread(void*) {
    LOGI(">>> Setting up hooks... <<<");
    sleep(3); 
    
    // Ставим хуки только на отрисовку
    int r1 = xhook_register(".*libUE4.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    int r2 = xhook_register(".*libUE4.so$", "eglSwapBuffersWithDamageKHR", (void*)hookSwapDamage, (void**)&origSwapDamage);
    
    if (r1 != 0) xhook_register(".*libEGL.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    if (r2 != 0) xhook_register(".*libEGL.so$", "eglSwapBuffersWithDamageKHR", (void*)hookSwapDamage, (void**)&origSwapDamage);

    xhook_refresh(0);
    LOGI("Hooks installed! (r1=%d, r2=%d)", r1, r2);
    
    return nullptr;
}

__attribute__((constructor))
void onLibraryLoaded() {
    LOGI(">>> Library injected! <<<");
    pthread_t thread;
    pthread_create(&thread, nullptr, SetupThread, nullptr);
}

extern "C" int main() { return 0; }

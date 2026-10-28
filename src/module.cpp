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
    if (dpy == nullptr || surf == nullptr) return;

    static bool inited = false;
    static int last_w = 0, last_h = 0;
    
    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, surf, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);
    
    if (w <= 0 || h <= 0) return;

    if (!inited || w != last_w || h != last_h) {
        LOGI("ImGui Init/Resize: %dx%d", w, h);
        if (!inited) {
            ImGuiInit(dpy, surf, w, h);
            inited = true;
        } else {
            ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);
        }
        last_w = w;
        last_h = h;
    }
    
    ImGuiBeginFrame();
    DrawESP();
    DrawMenu();
    ImGuiRender();
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
    LOGI(">>> Waiting for libUE4.so... <<<");
    
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
    LOGI(">>> Setting up hooks... <<<");
    
    xhook_register(".*libUE4.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    xhook_register(".*libUE4.so$", "eglSwapBuffersWithDamageKHR", (void*)hookSwapDamage, (void**)&origSwapDamage);
    xhook_register(".*libEGL.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    xhook_register(".*libEGL.so$", "eglSwapBuffersWithDamageKHR", (void*)hookSwapDamage, (void**)&origSwapDamage);
    
    xhook_refresh(0);
    LOGI("Hooks installed!");
    return nullptr;
}

__attribute__((constructor))
void onLibraryLoaded() {
    LOGI(">>> Library injected! <<<");
    pthread_t thread;
    pthread_create(&thread, nullptr, SetupThread, nullptr);
}

extern "C" int main() { return 0; }

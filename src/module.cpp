#include <jni.h>
#include <android/log.h>
#include <android/input.h>
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

// === ОТРИСОВКА ===
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

// === ТАЧ-ВВОД ===
using AInputQueue_getEvent_t = int32_t (*)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_getEvent = nullptr;

int32_t hook_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t ret = orig_getEvent(queue, outEvent);
    
    if (ret >= 0 && *outEvent) {
        if (ImGui::GetCurrentContext() != nullptr) {
            int32_t type = AInputEvent_getType(*outEvent);
            if (type == AINPUT_EVENT_TYPE_MOTION) {
                ImGuiIO& io = ImGui::GetIO();
                
                int32_t action = AMotionEvent_getAction(*outEvent);
                int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
                
                float x = AMotionEvent_getX(*outEvent, 0);
                float y = AMotionEvent_getY(*outEvent, 0);
                
                // Передаем события в ImGui
                if (maskedAction == AMOTION_EVENT_ACTION_DOWN) {
                    io.AddMousePosEvent(x, y);
                    io.AddMouseButtonEvent(0, true); // Палец нажат
                } else if (maskedAction == AMOTION_EVENT_ACTION_MOVE) {
                    io.AddMousePosEvent(x, y); // Палец двигается (таскаем окно)
                } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
                    io.AddMousePosEvent(x, y);
                    io.AddMouseButtonEvent(0, false); // Палец отпущен
                }
            }
        }
    }
    return ret;
}

// === ПОТОК УСТАНОВКИ ХУКОВ ===
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
    
    // Отрисовка
    xhook_register(".*libUE4.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    xhook_register(".*libUE4.so$", "eglSwapBuffersWithDamageKHR", (void*)hookSwapDamage, (void**)&origSwapDamage);
    xhook_register(".*libEGL.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    xhook_register(".*libEGL.so$", "eglSwapBuffersWithDamageKHR", (void*)hookSwapDamage, (void**)&origSwapDamage);
    
    // Тач
    xhook_register(".*libandroid.so$", "AInputQueue_getEvent", (void*)hook_getEvent, (void**)&orig_getEvent);
    xhook_register(".*libUE4.so$", "AInputQueue_getEvent", (void*)hook_getEvent, (void**)&orig_getEvent);
    
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

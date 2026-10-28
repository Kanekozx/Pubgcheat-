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

// Объявления функций из menu.cpp
void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h);
void ImGuiBeginFrame();
void DrawESP();
void DrawMenu();
void ImGuiRender();

// === ОТРИСОВКА (OpenGL) ===
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
    } else {
        ImGuiBeginFrame();
        DrawESP();
        DrawMenu();
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
                
                // Передаем позицию
                io.AddMousePosEvent(x, y);
                
                // Нажатие и отпускание
                if (maskedAction == AMOTION_EVENT_ACTION_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (maskedAction == AMOTION_EVENT_ACTION_UP || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
                    io.AddMouseButtonEvent(0, false);
                }
            }
        }
    }
    return ret;
}

// === ПОТОК УСТАНОВКИ ХУКОВ ===
void* SetupThread(void*) {
    LOGI(">>> Setting up hooks... <<<");
    sleep(3); 
    
    // Ставим хуки только на нужные библиотеки
    int r1 = xhook_register(".*libEGL.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    int r2 = xhook_register(".*libEGL.so$", "eglSwapBuffersWithDamageKHR", (void*)hookSwapDamage, (void**)&origSwapDamage);
    int r3 = xhook_register(".*libandroid.so$", "AInputQueue_getEvent", (void*)hook_getEvent, (void**)&orig_getEvent);
    
    if (r1 == 0 || r2 == 0 || r3 == 0) {
        xhook_refresh(0);
        LOGI("Hooks installed! (r1=%d, r2=%d, r3=%d)", r1, r2, r3);
    } else {
        LOGI("Hook FAILED!");
    }
    return nullptr;
}

__attribute__((constructor))
void onLibraryLoaded() {
    LOGI(">>> Library injected! <<<");
    pthread_t thread;
    pthread_create(&thread, nullptr, SetupThread, nullptr);
}

extern "C" int main() { return 0; }

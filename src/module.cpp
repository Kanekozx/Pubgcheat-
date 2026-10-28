#include <jni.h>
#include <android/log.h>
#include <android/input.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <xhook.h>
#include "imgui.h"

void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h);
void ImGuiBeginFrame();
void DrawESP();
void DrawMenu();
void ImGuiRender();

void log_to_file(const char* msg) {
    FILE* f = fopen("/data/local/tmp/cheat_log.txt", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

// === ХУК НА ОТРИСОВКУ ===
using eglSwapBuffers_t = EGLBoolean(*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t origSwap = nullptr;

static EGLBoolean hookSwap(EGLDisplay dpy, EGLSurface surf) {
    static bool inited = false;
    if (!inited) {
        EGLint _w = 0, _h = 0;
        eglQuerySurface(dpy, surf, EGL_WIDTH, &_w);
        eglQuerySurface(dpy, surf, EGL_HEIGHT, &_h);
        if (_w > 0 && _h > 0) {
            ImGuiInit(dpy, surf, _w, _h);
            inited = true;
        } else {
            return origSwap(dpy, surf);
        }
    }
    ImGuiBeginFrame();
    DrawESP();
    DrawMenu();
    ImGuiRender();
    return origSwap(dpy, surf);
}

// === ХУК НА ТАЧ (ВРУЧНУЮ) ===
using AInputQueue_getEvent_t = int32_t (*)(AInputQueue* queue, AInputEvent** outEvent);
static AInputQueue_getEvent_t orig_getEvent = nullptr;

int32_t hook_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
    int32_t ret = orig_getEvent(queue, outEvent);
    
    if (ret >= 0 && *outEvent) {
        int32_t type = AInputEvent_getType(*outEvent);
        if (type == AINPUT_EVENT_TYPE_MOTION) {
            ImGuiIO& io = ImGui::GetIO();
            int32_t action = AMotionEvent_getAction(*outEvent) & AMOTION_EVENT_ACTION_MASK;
            float x = AMotionEvent_getX(*outEvent, 0);
            float y = AMotionEvent_getY(*outEvent, 0);
            
            io.MousePos = ImVec2(x, y);
            if (action == AMOTION_EVENT_ACTION_DOWN) {
                io.MouseDown[0] = true;
            } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_CANCEL) {
                io.MouseDown[0] = false;
            }
        }
    }
    return ret;
}

// === ПОТОК ХУКОВ ===
void* SetupThread(void*) {
    log_to_file(">>> Thread started! <<<");
    sleep(5); // Ждем загрузки игры
    
    char buf[128];
    int r1 = xhook_register(".*libUE4.so$", "eglSwapBuffers", (void*)hookSwap, (void**)&origSwap);
    sprintf(buf, "eglSwapBuffers hook: %d", r1); log_to_file(buf);

    int r3 = xhook_register(".*libandroid.so$", "AInputQueue_getEvent", (void*)hook_getEvent, (void**)&orig_getEvent);
    sprintf(buf, "AInputQueue_getEvent hook: %d", r3); log_to_file(buf);
    
    if (r1 == 0 || r3 == 0) {
        xhook_refresh(0);
        log_to_file("xhook_refresh success!");
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

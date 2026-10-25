#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_android.h>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <cstring>
#include <cstdio>

#include <xhook.h>

#include "src/config.h"
#include "src/offsets.h"
#include "src/sdk.h"
#include "src/menu.h"
#include "src/esp.h"

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "pubgcheat", __VA_ARGS__)

static bool  g_imgui_ready = false;
static EGLBoolean (*orig_swap)(EGLDisplay, EGLSurface)  = nullptr;
static int        (*orig_input)(AInputQueue*, AInputEvent**) = nullptr;

// ---------- утилиты ----------
static bool is_lib_loaded(const char* name) {
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return false;
    char line[512]; bool found = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, name)) { found = true; break; }
    }
    fclose(f); return found;
}

static bool resolve_lib(const char* name, char* out, size_t outlen) {
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return false;
    char line[512]; bool found = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, name) && strstr(line, "r-xp")) {
            char* p = strrchr(line, '/');
            if (p) {
                snprintf(out, outlen, "%s", p);
                size_t l = strlen(out);
                while (l > 0 && (out[l-1]=='\n'||out[l-1]=='\r')) out[--l]=0;
                found = true; break;
            }
        }
    }
    fclose(f); return found;
}

// ---------- ImGui ----------
static void init_imgui_gl(EGLDisplay dpy, EGLSurface surf) {
    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, surf, EGL_WIDTH,  &w);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);

    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImGui_ImplAndroid_Init();
    LOG("[pubgcheat] ImGui init %dx%d", w, h);
}

// ---------- хуки ----------
static EGLBoolean hook_swap(EGLDisplay dpy, EGLSurface surf) {
    if (!g_imgui_ready) {
        init_imgui_gl(dpy, surf);
        g_imgui_ready = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    drawESP();
    if (g_cfg.menuOpen) drawMenu();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_swap(dpy, surf);
}

static int hook_input(AInputQueue* q, AInputEvent** ev) {
    int r = orig_input(q, ev);
    if (r >= 0 && *ev && g_cfg.menuOpen) {
        // сюда при желании — закрытие меню по тапу
    }
    return r;
}

// ---------- entry point для dlopen ----------
__attribute__((constructor))
static void ptrace_entry() {
    LOG("[pubgcheat] constructor: waiting for libUE4.so");

    for (int i = 0; i < 30; i++) {
        if (is_lib_loaded("libUE4.so")) break;
        sleep(1);
    }
    if (!is_lib_loaded("libUE4.so")) {
        LOG("[pubgcheat] libUE4 not found, abort");
        return;
    }
    LOG("[pubgcheat] libUE4 ready");

    if (!SDK::init()) {
        LOG("[pubgcheat] SDK init failed");
        return;
    }
    LOG("[pubgcheat] SDK init ok");

    char lib[256];

    if (resolve_lib("libEGL.so", lib, sizeof(lib))) {
        xhook_register(lib, "eglSwapBuffers", (void*)hook_swap, (void**)&orig_swap);
        LOG("[pubgcheat] hook %s!eglSwapBuffers", lib);
    }

    if (resolve_lib("libandroid.so", lib, sizeof(lib))) {
        xhook_register(lib, "AInputQueue_getEvent", (void*)hook_input, (void**)&orig_input);
        LOG("[pubgcheat] hook %s!AInputQueue_getEvent", lib);
    }

    xhook_refresh(0);
    LOG("[pubgcheat] hooks installed");

    // автооткрытие через 10 сек, как у zSilent
    std::thread([]{
        std::this_thread::sleep_for(std::chrono::seconds(10));
        g_cfg.menuOpen = true;
        LOG("[pubgcheat] menu auto-opened after 10s");
    }).detach();
}

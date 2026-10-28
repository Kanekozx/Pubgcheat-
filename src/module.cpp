#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>
#include <unistd.h>
#include "zygisk.hpp"
#include "src/xhook_api.h"
#include "src/config.h"
#include "src/sdk.h"
#include "src/menu.h"
#include "src/offsets.h"
#include <xhook.h>
#include <cmath>
#include <EGL/egl.h>
#include <EGL/eglext.h>

// Оффсет из свежего SDK
#define SHOOT_BULLET_OFFSET 0x6FF841C

// Указатель на оригинальную функцию
void (*orig_ShootBulletInner)(void* _this, void* Target, void* StartLoc, void* Velocity, void* Params);

// Наша функция-хук
void hook_ShootBulletInner(void* _this, void* Target, void* StartLoc, void* Velocity, void* Params) {
    // Если цель существует, можно скорректировать Velocity (вектор пули)
    // ВАЖНО: Чтобы это сработало, тебе нужно через SDK получить координаты головы Target.
    // Пример (псевдокод): 
    // if (Target) { FVector head = SDK::getBoneLocation(Target, "head"); ... }
    
    // Вызываем оригинал, чтобы игра не крашнулась
    orig_ShootBulletInner(_this, Target, StartLoc, Velocity, Params);
}

// Функция инициализации (вызови её в SDK::init() или в главном потоке)
void InitBulletTrack() {
    void* base = (void*)SDK::base(); // Твоя функция получения базы libUE4.so
    void* targetAddr = (void*)((uintptr_t)base + SHOOT_BULLET_OFFSET);
    
    xhook_register(".*libUE4.so$", "ShootBulletInner", (void*)hook_ShootBulletInner, (void**)&orig_ShootBulletInner);
    xhook_refresh(0);
}
// -------- Логирование (в logcat) --------
#define LOG(fmt, ...) \
    do { char _b[512]; snprintf(_b, sizeof(_b), "[pubgcheat] " fmt, ##__VA_ARGS__); \
         __android_log_write(ANDROID_LOG_INFO, "pubgcheat", _b); } while (0)
#include <android/log.h>

// -------- Хуки --------
using eglSwapBuffers_t = EGLBoolean(*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t origSwap = nullptr;

static EGLBoolean hookSwap(EGLDisplay dpy, EGLSurface surf) {
    // Инициализация ImGui + ESP — один раз
    static bool inited = false;
    static int w = 0, h = 0;
    if (!inited) {
        EGLint _w = 0, _h = 0;
        eglQuerySurface(dpy, surf, EGL_WIDTH,  &_w);
        eglQuerySurface(dpy, surf, EGL_HEIGHT, &_h);
        w = _w; h = _h;
        LOG("ImGui init %dx%d", w, h);
        ImGuiInit(dpy, surf, w, h);
        inited = true;
    }

    if (!SDK::init()) {
        return origSwap ? origSwap(dpy, surf) : EGL_FALSE;
    }

    // ESP (draws)
    DrawESP();
    DrawMenu();

    // Отрисовка ImGui поверх кадра
    ImGuiRender();

    return origSwap ? origSwap(dpy, surf) : EGL_FALSE;
}

// -------- Основная логика (бывший PubgModule) --------
class PubgModule {
public:
    void onLoad(zygisk::Api* api, void* /*env*/) {
        this->api = api;
    }

    void postAppSpecialize(void* /*args*/) {
        LOG("postAppSpecialize start");

        // Ждём libUE4.so (игра грузит его не сразу)
        int waited = 0;
        while (!is_lib_loaded("libUE4.so") && waited < 30) {
            sleep(1);
            waited++;
        }
        if (!is_lib_loaded("libUE4.so")) {
            LOG("libUE4.so not loaded after 30s, giving up");
            return;
        }
        LOG("libUE4.so loaded after %ds", waited);

        // Ставим хук на eglSwapBuffers
        void* handle = dlopen("libEGL.so", RTLD_NOW);
        if (!handle) {
            LOG("dlopen libEGL.so failed: %s", dlerror());
            return;
        }

        // Определяем dev/ino из dladdr
        Dl_info info{};
        if (!dladdr((void*)eglSwapBuffers, &info) || !info.dli_fname) {
            LOG("dladdr libEGL failed");
            return;
        }
        struct stat st{};
        if (stat(info.dli_fname, &st) != 0) {
            LOG("stat %s failed", info.dli_fname);
            return;
        }
        LOG("libEGL path=%s dev=%lx ino=%lx",
            info.dli_fname, (unsigned long)st.st_dev, (unsigned long)st.st_ino);

        api->pltHookRegister(st.st_dev, st.st_ino,
                             "eglSwapBuffers",
                             (void*)hookSwap,
                             (void**)&origSwap);
        api->pltHookCommit();
        LOG("hooks installed");

        // Автооткрытие меню через 10 секунд (как в zSilent)
        std::thread([]{
            std::this_thread::sleep_for(std::chrono::seconds(10));
            g_cfg.menuOpen = true;
            LOG("menu auto-opened after 10s");
        }).detach();
    }

private:
    zygisk::Api* api = nullptr;
};

// -------- dlopen entry point (вместо REGISTER_ZYGISK_MODULE) --------
__attribute__((constructor))
static void ptrace_entry() {
    LOG("=== ptrace_entry: .so injected via dlopen ===");

    // На Android 16 xhook может нуждаться в отключении SIGSEGV-протектора
    xhook_enable_sigsegv_protection(0);
    xhook_enable_debug(1);

    // Небольшая задержка, чтобы процесс игры успел инициализироваться
    sleep(1);

    auto* api = new PtraceApi();
    auto* mod = new PubgModule();
    mod->onLoad(api, nullptr);
    mod->postAppSpecialize(nullptr);

    LOG("=== ptrace_entry: done ===");
}

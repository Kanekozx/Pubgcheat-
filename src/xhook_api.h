#pragma once
#include <zygisk.hpp>
#include <xhook.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/sysmacros.h>
#include <android/log.h>
#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "PubgCheat", __VA_ARGS__)

// Реализация zygisk::Api поверх xhook.
// Zygisk-API использует (dev, ino) чтобы найти библиотеку,
// а xhook — имя библиотеки (например "libEGL.so").
// Эта обёртка парсит /proc/self/maps и находит имя по (dev, ino).
class PtraceApi : public zygisk::Api {
public:
    void pltHookRegister(dev_t dev, ino_t inode, const char* symbol,
                         void* fn, void** backup) override {
        char libname[256] = {0};
        if (!find_lib(dev, inode, libname, sizeof(libname))) {
            LOG("cannot resolve lib dev=%lx ino=%lx", (unsigned long)dev, (unsigned long)inode);
            return;
        }
        int r = xhook_register(libname, symbol, fn, backup);
        LOG("hook %s!%s -> %p (ret=%d)", libname, symbol, fn, r);
    }

    void pltHookCommit() override {
        int r = xhook_refresh(0);
        LOG("xhook_refresh ret=%d", r);
    }

    void setOption(zygisk::Option) override {}
    void pltHookExclude(dev_t, ino_t, const char*) override {}
    int  connectCompanion() override { return -1; }
    int  getCompanionFd() override { return -1; }

private:
    static bool find_lib(dev_t dev, ino_t inode, char* out, size_t outlen) {
        FILE* f = fopen("/proc/self/maps", "r");
        if (!f) return false;
        char line[512];
        bool ok = false;
        while (fgets(line, sizeof(line), f)) {
            unsigned int d_maj = 0, d_min = 0;
            unsigned long long ino = 0;
            char path[256] = {0};
            if (sscanf(line, "%*lx-%*lx %*s %*lx %x:%x %llu %255s",
                       &d_maj, &d_min, &ino, path) == 4) {
                dev_t cur = makedev(d_maj, d_min);
                if (cur == dev && (ino_t)ino == inode) {
                    snprintf(out, outlen, "%s", path);
                    ok = true;
                    break;
                }
            }
        }
        fclose(f);
        return ok;
    }
};

// Проверка, загружена ли библиотека в текущий процесс
static inline bool is_lib_loaded(const char* name) {
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return false;
    char line[512];
    bool ok = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, name)) { ok = true; break; }
    }
    fclose(f);
    return ok;
}

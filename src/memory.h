#pragma once
#include <cstdint>
#include <cstring>
#include <csignal>
#include <csetjmp>
#include <android/log.h>

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "espint", __VA_ARGS__)

namespace Mem {

inline sigjmp_buf g_jmp;
inline volatile sig_atomic_t g_active = 0;

inline void segvHandler(int) {
    if (g_active) { g_active = 0; siglongjmp(g_jmp, 1); }
    signal(SIGSEGV, SIG_DFL);
}

inline void install() {
    struct sigaction sa{};
    sa.sa_handler = segvHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
}

// --- reference-форма: read(addr, out) ---
template<typename T>
inline bool read(uintptr_t addr, T& out) {
    if (!addr) return false;
    if (sigsetjmp(g_jmp, 1) == 0) {
        g_active = 1;
        memcpy(&out, (const void*)addr, sizeof(T));
        g_active = 0;
        return true;
    }
    return false;
}

// --- value-форма: rd<T>(addr, def) ---
template<typename T>
inline T rd(uintptr_t addr, T def = T{}) {
    T v = def;
    read(addr, v);
    return v;
}

inline uintptr_t rdPtr(uintptr_t addr) { return rd<uintptr_t>(addr, 0); }

} // namespace Mem

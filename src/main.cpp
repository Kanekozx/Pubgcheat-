#include "common.hpp"
#include "mem.hpp"
#include "module.hpp"
#include "ue4.hpp"
#include "offsets.hpp"
#include "menu.hpp"
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <cstring>

static volatile sig_atomic_t g_stop = 0;
static void on_sig(int) { g_stop = 1; }

int main(int argc, char** argv) {
    signal(SIGINT, on_sig);
    signal(SIGTERM, on_sig);

    pid_t pid = 0;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--pid") && i + 1 < argc) pid = atoi(argv[++i]);
    }
    if (!pid) return 1;

    Proc mem(pid);
    auto ue4 = find_module(pid, "libUE4.so");
    if (!ue4) { printf("[-] libUE4.so not found\n"); return 2; }

    printf("[*] Base address: %p\n", (void*)ue4->base);

    while (!g_stop) {
        printf("\n=== NEW OFFSET TEST ===\n");

        // 1. Проверяем UEngine (0xf621d90)
        ptr gengine = mem.rd<ptr>(ue4->base + 0xf621d90);
        printf("UEngine (0xf621d90): %p\n", (void*)gengine);
        
        if (gengine) {
            ptr viewport = mem.rd<ptr>(gengine + 0x810);
            printf("  -> GameViewport (0x810): %p\n", (void*)viewport);
            if (viewport) {
                ptr world = mem.rd<ptr>(viewport + 0x78);
                printf("    -> World (0x78): %p\n", (void*)world);
            }
        }

        // 2. Проверяем ULocalPlayer (0xf5fbff0)
        ptr local_player = mem.rd<ptr>(ue4->base + 0xf5fbff0);
        printf("ULocalPlayer (0xf5fbff0): %p\n", (void*)local_player);
        
        if (local_player) {
            // Читаем PlayerController из ULocalPlayer (оффсет 0x30)
            ptr pc = mem.rd<ptr>(local_player + 0x30);
            printf("  -> PlayerController (0x30): %p\n", (void*)pc);
        }

        sleep(3);
    }
    return 0;
}

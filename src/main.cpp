#include "common.hpp"
#include "mem.hpp"
#include "module.hpp"
#include "ue4.hpp"
#include "offsets.hpp"
#include "esp.hpp"
#include "overlay.hpp"
#include "menu.hpp"
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <string>
#include <cstring>

static volatile sig_atomic_t g_stop = 0;
static void on_sig(int) { g_stop = 1; }

static ptr cached_gworld = 0;
static ptr cached_local_pc = 0;

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);

    signal(SIGINT, on_sig);
    signal(SIGTERM, on_sig);

    pid_t pid = 0;
    std::string offsets_path = "/data/local/tmp/offsets.json";
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--pid") && i + 1 < argc) pid = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--offsets") && i + 1 < argc) offsets_path = argv[++i];
    }
    if (!pid) return 1;

    Proc mem(pid);
    auto ue4 = find_module(pid, "libUE4.so");
    if (!ue4) { printf("[-] libUE4.so not found\n"); return 2; }
    
    printf("[*] libUE4.so found at 0x%llx\n", (unsigned long long)ue4->base);

    UEOffsets off = load_offsets(offsets_path);
    start_web_menu(); // Запускаем сервер для приложения

    EspConfig esp; 

    printf("[*] Starting main loop...\n");

    while (!g_stop) {
        if (!cached_gworld || !cached_local_pc) {
            ptr gengine = mem.rd<ptr>(ue4->base + 0xf621d90);
            if (gengine) {
                ptr viewport = mem.rd<ptr>(gengine + 0x810);
                if (viewport) {
                    ptr gworld = mem.rd<ptr>(viewport + 0x78);
                    if (gworld) {
                        ptr local_player = mem.rd<ptr>(ue4->base + 0xf5fbff0);
                        if (local_player) {
                            ptr local_pc = mem.rd<ptr>(local_player + 0x30);
                            if (local_pc) {
                                cached_gworld = gworld;
                                cached_local_pc = local_pc;
                                printf("[+] CACHED! World: 0x%llx, PC: 0x%llx\n", 
                                    (unsigned long long)cached_gworld, (unsigned long long)cached_local_pc);
                            }
                        }
                    }
                }
            }
            
            if (!cached_gworld) {
                usleep(500'000);
                continue;
            }
        }

        ptr persistent_level = mem.rd<ptr>(cached_gworld + off.UWorld_PersistentLevel);
        if (!persistent_level) {
            printf("[!] World changed. Resetting cache...\n");
            cached_gworld = 0;
            cached_local_pc = 0;
            usleep(500'000);
            continue;
        }

        esp.enabled = g_esp_enabled.load();

        Overlay dummy_ovl;
        
        // Только ESP!
        esp_tick(mem, *ue4, off, cached_gworld, 0, cached_local_pc, dummy_ovl, esp);

        usleep(1000 * 1000 / 60);
    }
    return 0;
}

#include "common.hpp"
#include "mem.hpp"
#include "module.hpp"
#include "ue4.hpp"
#include "offsets.hpp"
#include "esp.hpp"
#include "aimbot.hpp"
#include "overlay.hpp"
#include "menu.hpp"
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <string>
#include <cstring>

static volatile sig_atomic_t g_stop = 0;
static void on_sig(int) { g_stop = 1; }

uint64_t decrypt_ptr(uint64_t ptr) {
    if (!ptr) return 0;
    return ((ptr >> 32) & 0xFFFFFFFF) | ((ptr & 0xFFFFFFFF) << 32);
}

int main(int argc, char** argv) {
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
    if (!ue4) return 2;
    
    UEOffsets off = load_offsets(offsets_path);
    start_web_menu();

    while (!g_stop) {
        uint64_t enc_world = mem.rd<uint64_t>(ue4->base + 0xF0DAA54);
        uint64_t dec_world = decrypt_ptr(enc_world);
        ptr gworld = (ptr)dec_world;
        
        if (!gworld) { 
            printf("[-] 1. GWorld is NULL\n");
            sleep(1); 
            continue; 
        }

        ptr game_instance = mem.rd<ptr>(gworld + 0x470);
        if (!game_instance) { 
            printf("[-] 2. GameInstance is NULL (World: 0x%llx)\n", (unsigned long long)gworld);
            sleep(1); 
            continue; 
        }

        // Читаем LocalPlayers по оффсету 0x48 из твоего SDK
        ptr local_players_array = mem.rd<ptr>(game_instance + 0x48);
        if (!local_players_array) { 
            printf("[-] 3. LocalPlayers Array is NULL (GI: 0x%llx)\n", (unsigned long long)game_instance);
            sleep(1); 
            continue; 
        }

        ptr local_player = mem.rd<ptr>(local_players_array);
        if (!local_player) { 
            printf("[-] 4. LocalPlayer is NULL (Array: 0x%llx)\n", (unsigned long long)local_players_array);
            sleep(1); 
            continue; 
        }

        ptr local_pc = mem.rd<ptr>(local_player + 0x30);
        if (!local_pc) { 
            printf("[-] 5. LocalPC is NULL (Player: 0x%llx)\n", (unsigned long long)local_player);
            sleep(1); 
            continue; 
        }

        // Если всё ок
        printf("[+] SUCCESS! LocalPC: 0x%llx\n", (unsigned long long)local_pc);
        
        Overlay dummy_ovl;
        esp.enabled = g_esp_enabled.load();
        aim.enabled = g_aim_enabled.load();
        
        esp_tick(mem, *ue4, off, gworld, 0, local_pc, dummy_ovl, esp);
        aimbot_tick(mem, off, gworld, local_pc, dummy_ovl, aim);

        usleep(1000 * 1000 / 60);
    }
    return 0;
}

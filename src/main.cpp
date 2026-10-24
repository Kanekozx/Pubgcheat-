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
        
        if (gworld) {
            // Читаем разные оффсеты из твоего дампа
            ptr level = mem.rd<ptr>(gworld + 0x30);
            ptr game_instance = mem.rd<ptr>(gworld + 0x470); // Твой json: 0x470
            ptr game_state = mem.rd<ptr>(gworld + 0x428);
            ptr owning_gi_alt = mem.rd<ptr>(gworld + 0x468); // Твой json: 0x468

            printf("[*] GWorld: 0x%llx\n", (unsigned long long)gworld);
            printf("[*] Level (0x30): 0x%llx\n", (unsigned long long)level);
            printf("[*] GameInstance (0x470): 0x%llx\n", (unsigned long long)game_instance);
            printf("[*] GameState (0x428): 0x%llx\n", (unsigned long long)game_state);
            printf("[*] AltGI (0x468): 0x%llx\n", (unsigned long long)owning_gi_alt);

            if (game_instance) {
                ptr local_players = mem.rd<ptr>(game_instance + 0x38);
                printf("[*] LocalPlayers (0x38): 0x%llx\n", (unsigned long long)local_players);
            }
            printf("-----------------------------------\n");
        } else {
            printf("[-] Waiting for GWorld...\n");
        }
        sleep(3); // Ждем 3 секунды перед следующим выводом
    }
    return 0;
}

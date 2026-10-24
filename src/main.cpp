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
#include <vector>

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
    
    start_web_menu();

    while (!g_stop) {
        uint64_t enc_world = mem.rd<uint64_t>(ue4->base + 0xF0DAA54);
        uint64_t dec_world = decrypt_ptr(enc_world);
        ptr gworld = (ptr)dec_world;
        
        if (gworld) {
            ptr game_instance = mem.rd<ptr>(gworld + 0x470); 
            
            if (game_instance) {
                printf("\n[+] GameInstance: 0x%llx\n", (unsigned long long)game_instance);
                printf("[*] Testing LocalPlayers offsets:\n");
                
                // Перебираем возможные смещения для массива LocalPlayers
                std::vector<int> offsets = {0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70};
                
                for (int offset : offsets) {
                    ptr local_players_array = mem.rd<ptr>(game_instance + offset);
                    if (local_players_array) {
                        ptr local_player = mem.rd<ptr>(local_players_array);
                        if (local_player) {
                            // В ULocalPlayer PlayerController обычно на 0x30
                            ptr local_pc = mem.rd<ptr>(local_player + 0x30);
                            if (local_pc) {
                                printf("  Offset 0x%X: Array=0x%llx, Player=0x%llx, PC=0x%llx\n",
                                    offset, (unsigned long long)local_players_array, (unsigned long long)local_player, (unsigned long long)local_pc);
                            }
                        }
                    }
                }
            }
        }
        sleep(5); // Ждем 5 секунд перед следующим тестом
    }
    return 0;
}

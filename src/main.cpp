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
    
    start_web_menu();

    while (!g_stop) {
        printf("--- TEST 1: VIA GENGINE ---\n");
        ptr gengine = mem.rd<ptr>(ue4->base + 0xF0DA8EC);
        printf("GEngine: 0x%llx\n", (unsigned long long)gengine);
        
        if (gengine && gengine > 0x6000000000) {
            ptr viewport = mem.rd<ptr>(gengine + 0x810);
            printf("GameViewport (0x810): 0x%llx\n", (unsigned long long)viewport);
            
            if (viewport && viewport > 0x6000000000) {
                ptr world = mem.rd<ptr>(viewport + 0x78);
                printf("World (0x78): 0x%llx\n", (unsigned long long)world);
                
                if (world && world > 0x6000000000) {
                    ptr gi = mem.rd<ptr>(world + 0x470);
                    printf("GameInstance (0x470): 0x%llx\n", (unsigned long long)gi);
                    
                    if (gi && gi > 0x6000000000) {
                        // Пробуем найти LocalPlayers (0x48)
                        uint64_t lp = mem.rd<uint64_t>(gi + 0x48);
                        printf("LocalPlayers (0x48): 0x%llx\n", (unsigned long long)lp);
                    }
                }
            }
        }

        printf("\n--- TEST 2: VIA GWorld ---\n");
        uint64_t enc_world = mem.rd<uint64_t>(ue4->base + 0xF0DAA54);
        uint64_t dec_world = decrypt_ptr(enc_world);
        printf("Decrypted GWorld: 0x%llx\n", (unsigned long long)dec_world);
        
        if (dec_world && dec_world > 0x6000000000) {
            ptr world = (ptr)dec_world;
            ptr gi = mem.rd<ptr>(world + 0x470);
            printf("GameInstance (0x470): 0x%llx\n", (unsigned long long)gi);
            
            if (gi && gi > 0x6000000000) {
                // Проверяем флаг шифрования
                u8 flag = mem.rd<u8>(gi + 0x80);
                printf("Flag (0x80): %d\n", flag);
                
                // Читаем оба массива
                ptr enc_arr = mem.rd<ptr>(gi + 0x38);
                ptr norm_arr = mem.rd<ptr>(gi + 0x48);
                printf("Enc Array (0x38): 0x%llx\n", (unsigned long long)enc_arr);
                printf("Norm Array (0x48): 0x%llx\n", (unsigned long long)norm_arr);
            }
        }
        
        printf("-----------------------------------\n");
        sleep(3);
    }
    return 0;
}

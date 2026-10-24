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
    
    start_web_menu();

    while (!g_stop) {
        uint64_t enc_world = mem.rd<uint64_t>(ue4->base + 0xF0DAA54);
        uint64_t dec_world = decrypt_ptr(enc_world);
        ptr gworld = (ptr)dec_world;
        
        if (!gworld) { usleep(500'000); continue; }

        ptr game_instance = mem.rd<ptr>(gworld + 0x470);
        if (!game_instance) { usleep(500'000); continue; }

        // Читаем флаг шифрования из SDK (0x80)
        u8 use_encrypt = mem.rd<u8>(game_instance + 0x80);
        
        // Читаем TArray EncryptedLocalPlayers (0x38)
        // Структура TArray: [Указатель на данные (8 байт)] + [Кол-во (4 байта)]
        ptr enc_data = mem.rd<ptr>(game_instance + 0x38);
        u32 enc_count = mem.rd<u32>(game_instance + 0x40);

        // Читаем TArray LocalPlayers (0x48)
        ptr norm_data = mem.rd<ptr>(game_instance + 0x48);
        u32 norm_count = mem.rd<u32>(game_instance + 0x50);

        printf("\n=== GAME INSTANCE INFO ===\n");
        printf("GI Address: 0x%llx\n", (unsigned long long)game_instance);
        printf("Use Encrypt Flag (0x80): %d\n", use_encrypt);
        printf("Encrypted Array (0x38): Data=0x%llx, Count=%u\n", (unsigned long long)enc_data, enc_count);
        printf("Normal Array    (0x48): Data=0x%llx, Count=%u\n", (unsigned long long)norm_data, norm_count);

        if (norm_count > 0 && norm_data) {
            uint64_t raw_player = mem.rd<uint64_t>(norm_data);
            printf("First Normal Player Raw: 0x%llx\n", (unsigned long long)raw_player);
            printf("First Normal Player Dec: 0x%llx\n", (unsigned long long)decrypt_ptr(raw_player));
        }

        if (enc_count > 0 && enc_data) {
            uint64_t raw_enc_player = mem.rd<uint64_t>(enc_data);
            printf("First Encrypted Player Raw: 0x%llx\n", (unsigned long long)raw_enc_player);
        }

        printf("==========================\n");
        sleep(3); // Пауза 3 секунды
    }
    return 0;
}

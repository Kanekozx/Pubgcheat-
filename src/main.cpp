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

// Функция расшифровки указателя (свап 32-битных половин)
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
    if (!ue4) { printf("[-] libUE4.so not found\n"); return 2; }
    
    UEOffsets off = load_offsets(offsets_path);
    
    start_web_menu();

    EspConfig esp; 
    AimConfig aim;
    aim.fov_deg = 15.0f;

    while (!g_stop) {
        // Читаем зашифрованный GWorld (из твоего 偏移.h)
        uint64_t enc_world = mem.rd<uint64_t>(ue4->base + 0xF0DAA54);
        
        if (!enc_world) {
            usleep(500'000); 
            continue; 
        }

        // Расшифровываем его
        uint64_t dec_world = decrypt_ptr(enc_world);
        
        ptr gworld = (ptr)dec_world;

        static bool printed = false;
        if (!printed) {
            printf("[*] Encrypted GWorld: 0x%llx\n", enc_world);
            printf("[+] Decrypted UWorld: 0x%llx\n", dec_world);
            printed = true;
        }

        // Получаем GameInstance и LocalPlayer
        ptr game_instance = mem.rd<ptr>(gworld + off.UWorld_OwningGameInstance);
        if (!game_instance) { usleep(100'000); continue; }

        ptr local_players = mem.rd<ptr>(game_instance + 0x38);
        if (!local_players) { usleep(100'000); continue; }

        ptr local_player = mem.rd<ptr>(local_players);
        if (!local_player) { usleep(100'000); continue; }

        ptr local_pc = mem.rd<ptr>(local_player + 0x30);
        if (!local_pc) { usleep(100'000); continue; }

        static bool printed_ok = false;
        if (!printed_ok) { 
            printf("[+] LocalPlayerController found! Drawing...\n"); 
            printed_ok = true; 
        }

        esp.enabled = g_esp_enabled.load();
        aim.enabled = g_aim_enabled.load();

        Overlay dummy_ovl;
        esp_tick(mem, *ue4, off, gworld, 0, local_pc, dummy_ovl, esp);
        aimbot_tick(mem, off, gworld, local_pc, dummy_ovl, aim);

        usleep(1000 * 1000 / 60);
    }
    return 0;
}

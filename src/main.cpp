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
    
    // --- ВОТ ЭТА СТРОЧКА БЫЛА ПОТЕРЯНА ---
    UEOffsets off = load_offsets(offsets_path);
    // --------------------------------------
    
    start_web_menu();

    EspConfig esp;
    AimConfig aim;
    aim.fov_deg = 15.0f;

    while (!g_stop) {
        uint64_t enc_world = mem.rd<uint64_t>(ue4->base + 0xF0DAA54);
        uint64_t dec_world = decrypt_ptr(enc_world);
        ptr gworld = (ptr)dec_world;
        
        if (!gworld) { usleep(500'000); continue; }

        ptr game_instance = mem.rd<ptr>(gworld + 0x470);
        if (!game_instance) { usleep(500'000); continue; }

        ptr local_pc = 0;

        // --- ПОПЫТКА 1: Используем EncryptedLocalPlayers (0x38) ---
        ptr enc_array_data = mem.rd<ptr>(game_instance + 0x38);
        if (enc_array_data) {
            uint64_t enc_player = mem.rd<uint64_t>(enc_array_data);
            if (enc_player) {
                uint64_t dec_player = decrypt_ptr(enc_player);
                if (dec_player) {
                    local_pc = mem.rd<ptr>(dec_player + 0x30);
                    if (local_pc) {
                        printf("[+] SUCCESS via Encrypted Array! PC: 0x%llx\n", (unsigned long long)local_pc);
                    }
                }
            }
        }

        // --- ПОПЫТКА 2: Используем обычный LocalPlayers (0x48) ---
        if (!local_pc) {
            ptr normal_array = mem.rd<ptr>(game_instance + 0x48);
            if (normal_array) {
                uint64_t dec_array = decrypt_ptr((uint64_t)normal_array);
                ptr local_player = mem.rd<ptr>(dec_array);
                if (local_player) {
                    uint64_t dec_player = decrypt_ptr((uint64_t)local_player);
                    local_pc = mem.rd<ptr>(dec_player + 0x30);
                    if (local_pc) {
                        printf("[+] SUCCESS via Normal Array Decrypt! PC: 0x%llx\n", (unsigned long long)local_pc);
                    }
                }
            }
        }

        if (!local_pc) {
            static int retries = 0;
            if (retries++ % 60 == 0) {
                printf("[-] LocalPC still NULL. GI: 0x%llx\n", (unsigned long long)game_instance);
            }
            usleep(100'000);
            continue;
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

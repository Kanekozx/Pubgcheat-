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

ptr find_u_world_via_engine(const Proc& mem, ptr base, const UEOffsets& off) {
    // 1. Читаем GEngine (из твоего дампа Engine_Offset)
    ptr gengine = mem.rd<ptr>(base + 0xF0DA8EC);
    if (!gengine) {
        // Если пусто, возможно, дампер ошибся с оффсетом, пробуем GWorld (вдруг он не зашифрован сейчас)
        ptr gworld = mem.rd<ptr>(base + 0xF0DAA54);
        if (gworld) {
            printf("[*] GEngine is null, but GWorld is valid: %p\n", (void*)gworld);
            return gworld;
        }
        return 0;
    }

    // 2. UEngine -> GameViewport (оффсет 0x810)
    ptr game_viewport = mem.rd<ptr>(gengine + 0x810);
    if (!game_viewport) return 0;

    // 3. UGameViewportClient -> World (оффсет 0x78)
    ptr world = mem.rd<ptr>(game_viewport + 0x78);
    if (!world) return 0;

    printf("[+] Found UWorld via GEngine: %p (GEngine: %p)\n", (void*)world, (void*)gengine);
    return world;
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
        ptr gworld = find_u_world_via_engine(mem, ue4->base, off);
        
        if (!gworld) { 
            static int retries = 0;
            if (retries++ % 60 == 0) {
                printf("[-] UWorld not found yet. Are you in the lobby/menu? Retrying...\n");
            }
            usleep(100'000); 
            continue; 
        }

        // Дальше идем по стандартной цепочке
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

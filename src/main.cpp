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
#include <iostream>

static volatile sig_atomic_t g_stop = 0;
static void on_sig(int) { g_stop = 1; }

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
    if (!ue4) {
        std::cout << "[-] libUE4.so not found" << std::endl;
        return 2;
    }
    std::cout << "[+] libUE4.so base: " << std::hex << ue4->base << std::dec << std::endl;
    
    UEOffsets off = load_offsets(offsets_path);
    ptr gworld_ptr = ue4->base + off.GWorld;
    ptr gengine_ptr = ue4->base + off.Engine_LocalPlayer;

    std::cout << "[*] Starting web menu..." << std::endl;
    start_web_menu();

    EspConfig esp; 
    AimConfig aim;
    aim.fov_deg = 15.0f;

    int tick = 0;
    while (!g_stop) {
        ptr gworld = mem.rd<ptr>(gworld_ptr);
        ptr gengine = mem.rd<ptr>(gengine_ptr);

        if (tick++ % 60 == 0) {
            std::cout << "[*] GWorld: " << std::hex << gworld << ", GEngine: " << gengine << std::dec << std::endl;
        }

        if (!gworld || !gengine) { usleep(500'000); continue; }

        ptr local_players = mem.rd<ptr>(gengine + 0x38);
        if (!local_players) { usleep(100'000); continue; }
        ptr local_player = mem.rd<ptr>(local_players);
        if (!local_player) { usleep(100'000); continue; }
        ptr local_pc = mem.rd<ptr>(local_player + 0x30);
        if (!local_pc) { usleep(100'000); continue; }

        esp.enabled = g_esp_enabled.load();
        aim.enabled = g_aim_enabled.load();

        Overlay dummy_ovl;
        esp_tick(mem, *ue4, off, gworld, 0, local_pc, dummy_ovl, esp);
        aimbot_tick(mem, off, gworld, local_pc, dummy_ovl, aim);

        usleep(1000 * 1000 / 60);
    }
    return 0;
}

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
    if (!pid) { std::cout << "usage: " << argv[0] << " --pid <PUBG.pid>" << std::endl; return 1; }

    std::cout << "Starting cheat..." << std::endl;

    Proc mem(pid);
    auto ue4 = find_module(pid, "libUE4.so");
    if (!ue4) { std::cout << "libUE4.so not found" << std::endl; return 2; }
    
    UEOffsets off = load_offsets(offsets_path);
    ptr gworld_ptr = ue4->base + off.GWorld;
    ptr gengine_ptr = ue4->base + off.Engine_LocalPlayer;

    // Запускаем веб-меню
    start_web_menu();

    Overlay ovl;
    // ВРЕМЕННО ОТКЛЮЧАЕМ ГРАФИКУ
    // if (!ovl.init()) { std::cout << "overlay init failed" << std::endl; return 4; }
    std::cout << "Overlay disabled for test. Reading memory..." << std::endl;

    EspConfig esp; 
    AimConfig aim;
    aim.fov_deg = 15.0f;
    aim.smoothing = 4.0f;

    while (!g_stop) {
        ptr gworld = mem.rd<ptr>(gworld_ptr);
        ptr gengine = mem.rd<ptr>(gengine_ptr);

        if (!gworld || !gengine) { 
            usleep(500'000); 
            continue; 
        }

        ptr local_pc = 0;
        ptr local_players = mem.rd<ptr>(gengine + 0x38);
        if (local_players) {
            ptr local_player = mem.rd<ptr>(local_players);
            if (local_player) local_pc = mem.rd<ptr>(local_player + 0x30);
        }

        if (!local_pc) { 
            usleep(100'000); 
            continue; 
        }

        esp.enabled = g_esp_enabled.load();
        aim.enabled = g_aim_enabled.load();
        aim.draw_fov = g_aim_draw_fov.load();
        aim.fov_deg = g_aim_fov.load();
        aim.smoothing = g_aim_smooth.load();

        // ovl.begin();
        esp_tick(mem, *ue4, off, gworld, 0, local_pc, ovl, esp);
        aimbot_tick(mem, off, gworld, local_pc, ovl, aim);
        // ovl.end();

        usleep(1000 * 1000 / 60);
    }

    // ovl.shutdown();
    std::cout << "clean exit" << std::endl;
    return 0;
}

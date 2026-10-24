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

int main(int argc, char** argv) {
    // Отключаем буферизацию вывода, чтобы видеть все логи сразу
    setvbuf(stdout, NULL, _IONBF, 0);

    signal(SIGINT, on_sig);
    signal(SIGTERM, on_sig);

    pid_t pid = 0;
    std::string offsets_path = "/data/local/tmp/offsets.json";
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--pid") && i + 1 < argc) pid = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--offsets") && i + 1 < argc) offsets_path = argv[++i];
    }
    if (!pid) { printf("[-] No PID provided\n"); return 1; }

    Proc mem(pid);
    auto ue4 = find_module(pid, "libUE4.so");
    if (!ue4) { printf("[-] libUE4.so not found\n"); return 2; }
    
    printf("[*] libUE4.so found at 0x%llx\n", (unsigned long long)ue4->base);

    UEOffsets off = load_offsets(offsets_path);
    start_web_menu(); // Запускаем веб-сервер для меню

    EspConfig esp; 
    AimConfig aim;
    aim.fov_deg = 15.0f;

    printf("[*] Starting main loop...\n");

    while (!g_stop) {
        // 1. Получаем UEngine
        ptr gengine = mem.rd<ptr>(ue4->base + 0xf621d90);
        if (!gengine) { 
            printf("[-] UEngine is NULL\n");
            usleep(500'000); 
            continue; 
        }

        // 2. Получаем GameViewport
        ptr viewport = mem.rd<ptr>(gengine + 0x810);
        if (!viewport) { 
            printf("[-] GameViewport is NULL (Engine: 0x%llx)\n", (unsigned long long)gengine);
            usleep(500'000); 
            continue; 
        }

        // 3. Получаем UWorld
        ptr gworld = mem.rd<ptr>(viewport + 0x78);
        if (!gworld) { 
            printf("[-] UWorld is NULL (Viewport: 0x%llx)\n", (unsigned long long)viewport);
            usleep(500'000); 
            continue; 
        }

        // 4. Получаем ULocalPlayer
        ptr local_player = mem.rd<ptr>(ue4->base + 0xf5fbff0);
        if (!local_player) { 
            printf("[-] ULocalPlayer is NULL\n");
            usleep(500'000); 
            continue; 
        }

        // 5. Получаем PlayerController
        ptr local_pc = mem.rd<ptr>(local_player + 0x30);
        if (!local_pc) { 
            printf("[-] PlayerController is NULL (LocalPlayer: 0x%llx)\n", (unsigned long long)local_player);
            usleep(500'000); 
            continue; 
        }

        static bool printed_ok = false;
        if (!printed_ok) { 
            printf("[+] SUCCESS! World: 0x%llx, PC: 0x%llx\n", (unsigned long long)gworld, (unsigned long long)local_pc); 
            printed_ok = true; 
        }

        esp.enabled = g_esp_enabled.load();
        aim.enabled = g_aim_enabled.load();

        Overlay dummy_ovl;
        
        // Вызываем функции
        esp_tick(mem, *ue4, off, gworld, 0, local_pc, dummy_ovl, esp);
        aimbot_tick(mem, *ue4, off, gworld, local_pc, dummy_ovl, aim);

        usleep(1000 * 1000 / 60); // 60 FPS
    }
    return 0;
}

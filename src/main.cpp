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
#include <cmath>

static volatile sig_atomic_t g_stop = 0;
static void on_sig(int) { g_stop = 1; }

int main(int argc, char** argv) {
    signal(SIGINT, on_sig);
    signal(SIGTERM, on_sig);

    pid_t pid = 0;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--pid") && i + 1 < argc) pid = atoi(argv[++i]);
    }
    if (!pid) return 1;

    Proc mem(pid);
    auto ue4 = find_module(pid, "libUE4.so");
    if (!ue4) return 2;

    printf("[*] libUE4.so found at %p\n", (void*)ue4->base);

    while (!g_stop) {
        // 1. Получаем GWorld через UEngine (0xf621d90)
        ptr gengine = mem.rd<ptr>(ue4->base + 0xf621d90);
        if (!gengine) { usleep(500'000); continue; }
        ptr viewport = mem.rd<ptr>(gengine + 0x810);
        if (!viewport) { usleep(500'000); continue; }
        ptr gworld = mem.rd<ptr>(viewport + 0x78);
        if (!gworld) { usleep(500'000); continue; }

        // 2. Получаем PlayerController через ULocalPlayer (0xf5fbff0)
        ptr local_player = mem.rd<ptr>(ue4->base + 0xf5fbff0);
        if (!local_player) { usleep(500'000); continue; }
        ptr local_pc = mem.rd<ptr>(local_player + 0x30);
        if (!local_pc) { usleep(500'000); continue; }

        // 3. Ищем всех персонажей через GObjects
        ptr gobjects = ue4->base + 0xF360880; 
        ptr objects_ptr = mem.rd<ptr>(gobjects + 0x10);
        u32 num_elements = mem.rd<u32>(gobjects + 0x1C);

        if (!objects_ptr || num_elements == 0) {
            objects_ptr = mem.rd<ptr>(gobjects);
            num_elements = mem.rd<u32>(gobjects + 0x14);
        }

        if (objects_ptr && num_elements > 0) {
            int found_players = 0;
            
            // Ограничиваем скан для скорости
            u32 limit = std::min<u32>(num_elements, 200000);
            
            for (u32 i = 0; i < limit; ++i) {
                u32 chunk_idx = i / 0x10000;
                u32 offset = i % 0x10000;
                ptr chunk = mem.rd<ptr>(objects_ptr + chunk_idx * 8);
                if (!chunk) continue;
                ptr obj = mem.rd<ptr>(chunk + offset * 8);
                if (!obj) continue;

                // Читаем ClassPrivate объекта
                ptr class_ptr = mem.rd<ptr>(obj + 0x10);
                if (!class_ptr) continue;

                // Читаем имя класса через GNames (адрес из 4.6.0)
                ptr gnames_ptr = mem.rd<ptr>(ue4->base + 0xF08F830); // Твой GNames
                if (!gnames_ptr) continue;

                u32 name_idx = mem.rd<u32>(class_ptr + 0x18);
                ptr fname_chunk = mem.rd<ptr>(gnames_ptr + 0x10 + (name_idx >> 16) * 8);
                if (!fname_chunk) continue;
                ptr entry_ptr = mem.rd<ptr>(fname_chunk + (name_idx & 0xFFFF) * 8);
                if (!entry_ptr) continue;
                
                u16 header = mem.rd<u16>(entry_ptr);
                u32 len = (header >> 6) & 0x3FF;
                if (len == 0 || len > 64) continue;
                
                char class_name[64] = {0};
                mem.read(entry_ptr + 2, class_name, len);

                // Если это класс персонажа (ASTExtraPlayerCharacter)
                if (strstr(class_name, "PlayerCharacter") || strstr(class_name, "Character")) {
                    // Читаем здоровье (оффсет 0xE60 из твоего json)
                    float health = mem.rd<float>(obj + 0xE60);
                    if (health > 0.0f && health <= 200.0f) {
                        // Читаем координаты (RootComponent + 0x1E4)
                        ptr root = mem.rd<ptr>(obj + 0x208);
                        if (root) {
                            FVector loc;
                            if (mem.read(root + 0x1E4, &loc, sizeof(FVector))) {
                                found_players++;
                                if (found_players <= 5) {
                                    printf("[+] Found Player! Class: %s | HP: %.1f | Pos: %.1f, %.1f, %.1f\n", 
                                        class_name, health, loc.x, loc.y, loc.z);
                                }
                            }
                        }
                    }
                }
            }
            if (found_players > 0) {
                static int log_t = 0;
                if (log_t++ % 120 == 0) {
                    printf("[*] Total players found in scan: %d\n", found_players);
                }
            }
        }

        usleep(1000 * 1000 / 10); // 10 FPS для сканирования
    }
    return 0;
}

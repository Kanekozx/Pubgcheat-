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
#include <algorithm>

static volatile sig_atomic_t g_stop = 0;
static void on_sig(int) { g_stop = 1; }

ptr find_u_world(const Proc& mem, ptr base, const UEOffsets& off) {
    ptr gobjects = base + off.GObjects;
    ptr objects_ptr = mem.rd<ptr>(gobjects + 0x10);
    u32 num_elements = mem.rd<u32>(gobjects + 0x1C);
    
    if (!objects_ptr || num_elements == 0) {
        objects_ptr = mem.rd<ptr>(gobjects);
        num_elements = mem.rd<u32>(gobjects + 0x14);
    }

    if (!objects_ptr || num_elements == 0) return 0;
    
    // ВАЖНО: Читаем указатель на FNamePool / TNameEntryArray
    ptr gnames_ptr = mem.rd<ptr>(base + off.GNames);
    if (!gnames_ptr) {
        printf("[-] GNames pointer is null!\n");
        return 0;
    }
    
    u32 limit = 50000; // TheWorld находится на индексе 536, так что 50к хватит с головой

    printf("[*] Scanning up to %u elements...\n", limit);
    int valid_strings = 0;

    for (u32 i = 0; i < limit; ++i) {
        u32 chunk_idx = i / 0x10000;
        u32 offset = i % 0x10000;
        ptr chunk = mem.rd<ptr>(objects_ptr + chunk_idx * 8);
        if (!chunk) continue;
        ptr obj = mem.rd<ptr>(chunk + offset * 8);
        if (!obj) continue;

        u32 name_idx = mem.rd<u32>(obj + 0x18);
        u32 fname_chunk_idx = name_idx >> 16;
        u32 fname_offset = name_idx & 0xFFFF;
        
        // TStaticIndirectArrayThreadSafeRead (массив указателей)
        ptr fname_chunk = mem.rd<ptr>(gnames_ptr + 0x10 + fname_chunk_idx * 8);
        if (!fname_chunk) continue;

        // Читаем указатель на саму строку (FNameEntry)
        ptr entry_ptr = mem.rd<ptr>(fname_chunk + fname_offset * 8);
        if (!entry_ptr) continue;

        u16 header = mem.rd<u16>(entry_ptr);
        u32 len = (header >> 6) & 0x3FF;
        bool is_wide = header & 1;

        if (len > 0 && len < 64) {
            char name[64] = {0};
            if (is_wide) {
                u16 wbuf[64];
                mem.read(entry_ptr + 2, wbuf, len * 2);
                for (u32 j = 0; j < len; ++j) name[j] = (char)wbuf[j];
            } else {
                mem.read(entry_ptr + 2, name, len);
            }
            
            // Фильтр от мусора
            bool printable = true;
            for (u32 j = 0; j < len; ++j) {
                if (name[j] < 32 || name[j] > 126) { printable = false; break; }
            }

            if (printable) {
                if (valid_strings < 20) {
                    printf("[DBG] Idx %u: %s\n", i, name);
                    valid_strings++;
                }
                if (strcmp(name, "TheWorld") == 0 || strcmp(name, "World") == 0) {
                    printf("[+] FOUND UWorld at index %u: %p\n", i, (void*)obj);
                    return obj;
                }
            }
        }
    }
    printf("[-] Scan finished. UWorld not found.\n");
    return 0;
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
        ptr gworld = find_u_world(mem, ue4->base, off);
        if (!gworld) { 
            printf("[-] Retrying in 1 sec...\n");
            sleep(1); 
            continue; 
        }

        ptr game_instance = mem.rd<ptr>(gworld + off.UWorld_OwningGameInstance);
        if (!game_instance) { usleep(100'000); continue; }

        ptr local_players = mem.rd<ptr>(game_instance + 0x38);
        if (!local_players) { usleep(100'000); continue; }

        ptr local_player = mem.rd<ptr>(local_players);
        if (!local_player) { usleep(100'000); continue; }

        ptr local_pc = mem.rd<ptr>(local_player + 0x30);
        if (!local_pc) { usleep(100'000); continue; }

        static bool printed_ok = false;
        if (!printed_ok) { printf("[+] LocalPlayerController found! Drawing...\n"); printed_ok = true; }

        esp.enabled = g_esp_enabled.load();
        aim.enabled = g_aim_enabled.load();

        Overlay dummy_ovl;
        esp_tick(mem, *ue4, off, gworld, 0, local_pc, dummy_ovl, esp);
        aimbot_tick(mem, off, gworld, local_pc, dummy_ovl, aim);

        usleep(1000 * 1000 / 60);
    }
    return 0;
}

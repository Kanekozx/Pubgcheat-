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

void read_fname(const Proc& mem, ptr entry_ptr, char* out, size_t max_len) {
    if (!entry_ptr) return;
    u16 header = mem.rd<u16>(entry_ptr);
    u32 len = (header >> 6) & 0x3FF; 
    bool is_wide = header & 1;

    if (len > 0 && len < max_len) {
        if (is_wide) {
            u16 wbuf[128];
            mem.read(entry_ptr + 2, wbuf, len * 2);
            for (u32 j = 0; j < len; ++j) out[j] = (char)wbuf[j];
        } else {
            mem.read(entry_ptr + 2, out, len);
        }
        out[len] = '\0';
    }
}

ptr find_u_world(const Proc& mem, ptr base, const UEOffsets& off) {
    ptr gobjects = base + off.GObjects;
    ptr objects_ptr = mem.rd<ptr>(gobjects + 0x10);
    u32 num_elements = mem.rd<u32>(gobjects + 0x1C);
    
    if (!objects_ptr || num_elements == 0) {
        objects_ptr = mem.rd<ptr>(gobjects);
        num_elements = mem.rd<u32>(gobjects + 0x14);
    }

    if (!objects_ptr || num_elements == 0) return 0;

    // Берем индекс 536 из твоего дампа (TheWorld)
    u32 i = 536; 
    u32 chunk_idx = i / 0x10000;
    u32 offset = i % 0x10000;
    ptr chunk = mem.rd<ptr>(objects_ptr + chunk_idx * 8);
    if (!chunk) return 0;
    
    ptr obj = mem.rd<ptr>(chunk + offset * 8);
    if (!obj) return 0;

    u32 name_idx = mem.rd<u32>(obj + 0x18);
    u32 fname_chunk_idx = name_idx >> 16;
    u32 fname_offset = name_idx & 0xFFFF;
    printf("[*] Object 536 NameIdx: 0x%X (Chunk %u, Offset %u)\n", name_idx, fname_chunk_idx, fname_offset);

    char name[128];

    // Метод 1: FNamePool лежит прямо по адресу base + GNames (Blocks на +0x10)
    ptr pool = base + off.GNames;
    ptr block1 = mem.rd<ptr>(pool + 0x10 + fname_chunk_idx * 8);
    if (block1) {
        memset(name, 0, sizeof(name));
        read_fname(mem, block1 + fname_offset * 2, name, sizeof(name));
        printf("[M1] Pool+0x10: %s\n", name);
    }

    // Метод 2: TNameEntryArray (указатель на массив чанков на +0x0)
    ptr array_ptr = mem.rd<ptr>(base + off.GNames);
    if (array_ptr) {
        ptr chunks = mem.rd<ptr>(array_ptr);
        if (chunks) {
            ptr block2 = mem.rd<ptr>(chunks + fname_chunk_idx * 8);
            if (block2) {
                ptr entry2 = mem.rd<ptr>(block2 + fname_offset * 8);
                if (entry2) {
                    memset(name, 0, sizeof(name));
                    read_fname(mem, entry2, name, sizeof(name));
                    printf("[M2] TNameArray+0x0: %s\n", name);
                }
            }
        }
    }

    // Метод 3: FNamePool по указателю (base + GNames - это указатель на пул)
    ptr pool3 = mem.rd<ptr>(base + off.GNames);
    if (pool3) {
        ptr block3 = mem.rd<ptr>(pool3 + 0x10 + fname_chunk_idx * 8);
        if (block3) {
            memset(name, 0, sizeof(name));
            read_fname(mem, block3 + fname_offset * 2, name, sizeof(name));
            printf("[M3] PoolPtr+0x10: %s\n", name);
        }
    }

    // Метод 4: TNameEntryArray (указатель на массив чанков на +0x8)
    if (array_ptr) {
        ptr chunks = mem.rd<ptr>(array_ptr + 0x8);
        if (chunks) {
            ptr block4 = mem.rd<ptr>(chunks + fname_chunk_idx * 8);
            if (block4) {
                ptr entry4 = mem.rd<ptr>(block4 + fname_offset * 8);
                if (entry4) {
                    memset(name, 0, sizeof(name));
                    read_fname(mem, entry4, name, sizeof(name));
                    printf("[M4] TNameArray+0x8: %s\n", name);
                }
            }
        }
    }

    return obj; // Временно возвращаем, чтобы остановить цикл
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

        // Дальнейший код пока пропустим, нам главное увидеть правильный [M]
        sleep(2);
    }
    return 0;
}

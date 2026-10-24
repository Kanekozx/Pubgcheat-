#include "esp.hpp"
#include "menu.hpp"
#include <cmath>
#include <vector>
#include <cstdio>
#include <mutex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct W2SResult { bool visible; float x, y, dist; };

static bool world_to_screen(const FVector& world, const FMinimalViewInfo& pov,
                            float screen_w, float screen_h, W2SResult& out) {
    FVector d = { world.x - pov.Location.x, world.y - pov.Location.y, world.z - pov.Location.z };
    float cy = cosf(pov.Rotation.yaw * (float)M_PI / 180.f);
    float sy = sinf(pov.Rotation.yaw * (float)M_PI / 180.f);
    float cp = cosf(pov.Rotation.pitch * (float)M_PI / 180.f);
    float sp = sinf(pov.Rotation.pitch * (float)M_PI / 180.f);

    float fwd   =  d.x * cp * cy + d.y * cp * sy - d.z * sp;
    float right = -d.x * sy       + d.y * cy;
    float up    =  d.x * sp * cy + d.y * sp * sy + d.z * cp;

    if (fwd < 1.f) { out.visible = false; return false; }
    float f = (screen_h * 0.5f) / tanf(pov.FOV * 0.5f * (float)M_PI / 180.f);
    out.x = screen_w * 0.5f + (right / fwd) * f;
    out.y = screen_h * 0.5f - (up    / fwd) * f;
    out.dist = std::sqrt(d.x*d.x + d.y*d.y + d.z*d.z) / 100.f;
    out.visible = true;
    return true;
}

static void read_pov(const Proc& mem, ptr local_pc, const UEOffsets& off, FMinimalViewInfo& pov, bool& ok) {
    ok = false;
    ptr cam = mem.rd<ptr>(local_pc + off.APlayerController_PlayerCameraManager);
    if (!cam) return;
    ptr cache = cam + off.APlayerCameraManager_Cache;
    if (!mem.read(cache + off.CameraCache_POV + off.MinimalViewInfo_Location, &pov.Location, sizeof(FVector))) return;
    if (!mem.read(cache + off.CameraCache_POV + off.MinimalViewInfo_Rotation, &pov.Rotation, sizeof(FRotator))) return;
    if (!mem.read(cache + off.CameraCache_POV + off.MinimalViewInfo_FOV, &pov.FOV, sizeof(float))) return;
    ok = true;
}

void esp_tick(const Proc& mem, const Module& ue4, const UEOffsets& off,
              ptr gworld, ptr gnames, ptr local_pc,
              Overlay& ovl, const EspConfig& cfg) {
    std::lock_guard<std::mutex> lock(g_draw_mutex);
    g_draw_list.clear();

    if (!cfg.enabled || !gworld || !local_pc) return;

    FMinimalViewInfo pov{};
    bool have_pov = false;
    read_pov(mem, local_pc, off, pov, have_pov);
    if (!have_pov) return;

    static int log_timer = 0;
    // Выводим камеру раз в секунду
    if (log_timer++ % 60 == 0) {
        printf("[ESP] CAM: X=%.1f Y=%.1f Z=%.1f | P=%.1f Y=%.1f | FOV=%.1f\n",
            pov.Location.x, pov.Location.y, pov.Location.z,
            pov.Rotation.pitch, pov.Rotation.yaw, pov.FOV);
    }

    ptr level = mem.rd<ptr>(gworld + off.UWorld_PersistentLevel);
    if (!level) return;
    ptr actors_ptr = mem.rd<ptr>(level + off.ULevel_Actors);
    u32 count = mem.rd<u32>(level + off.ULevel_ActorCount);
    if (!actors_ptr || count > 100000) return;

    ptr my_pawn = mem.rd<ptr>(local_pc + off.APlayerController_AcknowledgedPawn);
    
    // Разрешение экрана. Если не совпадает - рамки улетят.
    float sw = 3048.0f, sh = 2032.0f; 

    int found = 0;
    for (u32 i = 0; i < count; ++i) {
        ptr actor = mem.rd<ptr>(actors_ptr + i * 8);
        if (!actor || actor == my_pawn) continue;

        // ВРЕМЕННО УБРАЛИ ПРОВЕРКУ ЗДОРОВЬЯ, ЧТОБЫ УВИДЕТЬ ВСЕ ОБЪЕКТЫ
        ptr root = mem.rd<ptr>(actor + off.AActor_RootComponent);
        if (!root) continue;
        
        FVector loc;
        if (!mem.read(root + off.USceneComponent_RelativeLocation, &loc, sizeof(FVector))) continue;

        W2SResult s;
        if (!world_to_screen(loc, pov, sw, sh, s) || s.dist > cfg.max_distance) continue;

        // Выводим первые 3 объекта для проверки
        if (found < 3 && log_timer % 60 == 0) {
            printf("[ESP] Actor: Pos=(%.1f, %.1f, %.1f) -> Screen=(%.1f, %.1f) Dist=%.1fm\n",
                loc.x, loc.y, loc.z, s.x, s.y, s.dist);
            found++;
        }

        float box_h = 5000.f / s.dist;
        float box_w = box_h * 0.5f;
        BoxData bd;
        bd.x = s.x - box_w * 0.5f;
        bd.y = s.y - box_h;
        bd.w = box_w;
        bd.h = box_h;
        bd.text = "";
        g_draw_list.push_back(bd);
    }
}

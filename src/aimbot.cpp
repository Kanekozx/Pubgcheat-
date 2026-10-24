#include "aimbot.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void angle_to(const FVector& src, const FVector& dst, FRotator& out) {
    float dx = dst.x - src.x, dy = dst.y - src.y, dz = dst.z - src.z;
    float hyp = std::sqrt(dx*dx + dy*dy);
    out.pitch = std::atan2(dz, hyp) * (float)(180.0 / M_PI);
    out.yaw   = std::atan2(dy, dx)  * (float)(180.0 / M_PI);
    out.roll  = 0;
}

static void normalize(FRotator& r) {
    while (r.pitch >  89.f) r.pitch -= 180.f;
    while (r.pitch < -89.f) r.pitch += 180.f;
    while (r.yaw   > 180.f) r.yaw   -= 360.f;
    while (r.yaw   < -180.f) r.yaw  += 360.f;
}

static float fov_delta(const FRotator& a, const FRotator& b) {
    float dp = a.pitch - b.pitch, dy = a.yaw - b.yaw;
    if (dy >  180.f) dy -= 360.f;
    if (dy < -180.f) dy += 360.f;
    return std::sqrt(dp*dp + dy*dy);
}

void aimbot_tick(const Proc& mem, const Module& ue4, const UEOffsets& off,
                 ptr gworld, ptr local_pc,
                 Overlay& ovl, const AimConfig& cfg) {
    if (!cfg.enabled || !gworld || !local_pc) return;

    ptr cam = mem.rd<ptr>(local_pc + off.APlayerController_PlayerCameraManager);
    if (!cam) return;
    FVector my_loc;
    FRotator my_rot;
    
    if (!mem.read(cam + off.APlayerCameraManager_Cache + off.CameraCache_POV + off.MinimalViewInfo_Location,
                  &my_loc, sizeof(FVector))) return;
    if (!mem.read(cam + off.APlayerCameraManager_Cache + off.CameraCache_POV + off.MinimalViewInfo_Rotation,
                  &my_rot, sizeof(FRotator))) return;

    ptr level = mem.rd<ptr>(gworld + off.UWorld_PersistentLevel);
    if (!level) return;
    ptr actors_ptr = mem.rd<ptr>(level + off.ULevel_Actors);
    u32 count = mem.rd<u32>(level + off.ULevel_ActorCount);
    if (!actors_ptr || count > 100000) return;

    ptr my_pawn = mem.rd<ptr>(local_pc + off.APlayerController_AcknowledgedPawn);

    float best_fov = cfg.fov_deg;
    FRotator best_rot{}; bool have = false;

    for (u32 i = 0; i < count; ++i) {
        ptr actor = mem.rd<ptr>(actors_ptr + i * 8);
        if (!actor || actor == my_pawn) continue;

        // ПРОВЕРКА НА ИГРОКА
        if (!is_valid_player(mem, ue4, off, actor)) continue;

        ptr root = mem.rd<ptr>(actor + off.AActor_RootComponent);
        if (!root) continue;
        
        FVector loc;
        if (!mem.read(root + off.USceneComponent_RelativeLocation, &loc, sizeof(FVector))) continue;

        loc.z += 60.0f; // Смещение в голову

        float dx = loc.x - my_loc.x, dy = loc.y - my_loc.y, dz = loc.z - my_loc.z;
        if (std::sqrt(dx*dx + dy*dy + dz*dz) / 100.f > cfg.max_dist) continue;

        FRotator target; angle_to(my_loc, loc, target); normalize(target);
        float f = fov_delta(my_rot, target);
        if (f < best_fov) { best_fov = f; best_rot = target; have = true; }
    }

    if (!have) return;

    FRotator blended;
    blended.pitch = my_rot.pitch + (best_rot.pitch - my_rot.pitch) / cfg.smoothing;
    blended.yaw   = my_rot.yaw   + (best_rot.yaw   - my_rot.yaw  ) / cfg.smoothing;
    blended.roll  = 0;
    normalize(blended);

    mem.wr<FRotator>(local_pc + off.APlayerController_ControlRotation, blended);
}

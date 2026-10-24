#pragma once
#include "mem.hpp"
#include "module.hpp" // <--- Добавили модуль
#include "ue4.hpp"
#include "overlay.hpp"

struct AimConfig {
    bool  enabled     = true;
    float fov_deg     = 8.0f;
    float smoothing   = 6.0f;
    float max_dist    = 300.f;
    bool  draw_fov    = true;
};

// Добавили const Module& ue4
void aimbot_tick(const Proc& mem, const Module& ue4, const UEOffsets& off,
                 ptr gworld, ptr local_pc,
                 Overlay& ovl, const AimConfig& cfg);

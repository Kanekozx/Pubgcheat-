#pragma once
#include "mem.hpp"
#include "ue4.hpp"
#include "overlay.hpp"

struct AimConfig {
    bool  enabled     = true;
    float fov_deg     = 8.0f;
    float smoothing   = 6.0f;
    float max_dist    = 300.f;
    bool  draw_fov    = true;
};

void aimbot_tick(const Proc& mem, const UEOffsets& off,
                 ptr gworld, ptr local_pc,
                 Overlay& ovl, const AimConfig& cfg);

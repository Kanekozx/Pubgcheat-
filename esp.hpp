#pragma once
#include "mem.hpp"
#include "module.hpp"
#include "ue4.hpp"
#include "overlay.hpp"

struct EspConfig {
    bool  enabled      = true;
    bool  boxes        = true;
    bool  names        = true;
    bool  health       = true;
    bool  distance     = true;
    float max_distance = 300.f;
    u32   color_ally   = 0xFF00FF00;
    u32   color_enemy  = 0xFFFF2020;
};

void esp_tick(const Proc& mem, const Module& ue4, const UEOffsets& off,
              ptr gworld, ptr gnames, ptr local_pc,
              Overlay& ovl, const EspConfig& cfg);

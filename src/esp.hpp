#pragma once
#include "mem.hpp"
#include "module.hpp"
#include "ue4.hpp"
#include "overlay.hpp"

struct EspConfig {
    bool  enabled      = true;
    bool  boxes        = true;
    float max_distance = 300.f;
};

void esp_tick(const Proc& mem, const Module& ue4, const UEOffsets& off,
              ptr gworld, ptr gnames, ptr local_pc,
              Overlay& ovl, const EspConfig& cfg);

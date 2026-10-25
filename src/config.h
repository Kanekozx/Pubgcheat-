#pragma once

struct Config {
    bool  menuOpen  = true;   
    bool  enabled   = true;
    bool  box       = true;
    bool  health    = true;
    bool  distance  = true;
    bool  snaplines = false;

    float maxDist   = 400.f;
    float boxColor[3] = { 1.f, 0.2f, 0.2f };
};

inline Config g_cfg;

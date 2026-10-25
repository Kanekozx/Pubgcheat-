#pragma once
#include <cstdint>
#include <vector>

using ptr = uintptr_t;

struct FVector  { float X, Y, Z; };
struct FRotator { float Pitch, Yaw, Roll; };

struct FMinimalViewInfo {
    FVector  Location;      // 0x00
    uint8_t  pad0[0xC];     // 0x0C
    FRotator Rotation;      // 0x18
    float    FOV;           // 0x24
};

namespace SDK {

bool  init();                       // найти base libUE4.so
ptr   base();

bool  getCamera(FMinimalViewInfo& pov);
ptr   getLocalPawn();
bool  isValidPlayer(ptr actor);

struct PlayerInfo {
    ptr     actor;
    FVector world;
    float   hp;
    float   dist;
};
bool  collectPlayers(std::vector<PlayerInfo>& out, int maxCount = 200);

bool  worldToScreen(const FVector& w, const FMinimalViewInfo& pov,
                    int sw, int sh, float& sx, float& sy);

}

#pragma once
#include <stdint.h>
#include <vector>
#include <dlfcn.h>

struct FVector { 
    float X, Y, Z; 
    FVector operator-(const FVector& v) const { return {X - v.X, Y - v.Y, Z - v.Z}; }
};

struct FRotator { float Pitch, Yaw, Roll; };

struct FMinimalViewInfo {
    FVector Location;
    FRotator Rotation;
    float FOV;
};

struct PlayerInfo {
    uintptr_t Actor;
    FVector Location;
    float Distance;
};

// Функции SDK
void* getModuleBase(const char* name);
bool getCamera(FMinimalViewInfo& pov);
std::vector<PlayerInfo> GetPlayers();
bool WorldToScreen(FVector WorldLocation, FMinimalViewInfo cam, FVector& ScreenLocation, int ScreenW, int ScreenH);

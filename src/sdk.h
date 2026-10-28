#pragma once

#include <stdint.h>
#include <vector>
#include <dlfcn.h>

struct FVector { float X, Y, Z; };
struct FRotator { float Pitch, Yaw, Roll; };

struct FMinimalViewInfo {
    FVector Location;
    FRotator Rotation;
    float FOV;
    float DesiredFOV;
};

struct PlayerInfo {
    uintptr_t Actor;
    FVector Location;
    float Health;
    float Distance;
};

// Оставляем только те функции, которые нужны другим файлам (menu.cpp)
void* getModuleBase(const char* name);
bool collectPlayers(std::vector<PlayerInfo>& out, int maxCount);
bool worldToScreen(const FVector& w, const FMinimalViewInfo& pov, FVector& screen, int width, int height);

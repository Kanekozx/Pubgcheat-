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

// Добавили Health, чтобы push_back({actor, loc, hp, dist}) не ругался
struct PlayerInfo {
    uintptr_t Actor;
    FVector Location;
    float Health;   // <--- ТЕПЕРЬ ЗДЕСЬ 4 ПОЛЯ
    float Distance;
};

// Объявления функций
void* getModuleBase(const char* name);
bool getCamera(FMinimalViewInfo& pov);
bool collectPlayers(std::vector<PlayerInfo>& out, int maxCount);
bool worldToScreen(const FVector& w, const FMinimalViewInfo& pov, FVector& screen, int width, int height);

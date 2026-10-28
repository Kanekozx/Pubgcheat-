#pragma once

#include <stdint.h>
#include <vector>
#include <dlfcn.h>

// Базовые структуры UE4
struct FVector { float X, Y, Z; };
struct FRotator { float Pitch, Yaw, Roll; };

// Структура для камеры
struct FMinimalViewInfo {
    FVector Location;
    FRotator Rotation;
    float FOV;
    float DesiredFOV;
};

// Структура для игрока
struct PlayerInfo {
    uintptr_t Actor;      // Адрес актора в памяти
    FVector Location;     // Координаты в мире
    float Distance;       // Дистанция до игрока
    // Если твой sdk.cpp требует другие поля (например, Health или Name), добавь их здесь
};

// Объявления функций (они должны совпадать с тем, что написано в sdk.cpp)
void* getModuleBase(const char* name);
bool getCamera(FMinimalViewInfo& pov);
bool collectPlayers(std::vector<PlayerInfo>& out, int maxCount);
bool worldToScreen(const FVector& w, const FMinimalViewInfo& pov, FVector& screen, int width, int height);

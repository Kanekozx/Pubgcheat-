#pragma once

#include <stdint.h>
#include <vector>
#include <dlfcn.h>

// Базовые структуры UE4
struct FVector { float X, Y, Z; };
struct FRotator { float Pitch, Yaw, Roll; };

// Функции, которые мы будем использовать в menu.cpp
bool WorldToScreen(FVector WorldLocation, FVector& ScreenLocation, int width, int height);
std::vector<uintptr_t> GetPlayers();
void* getModuleBase(const char* name);

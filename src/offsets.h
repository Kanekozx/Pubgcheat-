#pragma once
#include <cstdint>
using ptr = uintptr_t;

namespace Off {
    // --- Глобалы (свежие оффсеты 4.6.0) ---
    constexpr ptr GEngine = 0xF621D90;
    constexpr ptr GNames = 0x8BE1BFC;       // Обновлено
    constexpr ptr GObjects = 0xF360880;
    constexpr ptr GWorld = 0xF621EF8;       // Обновлено (World_Offset)
    
    // --- Путь через GEngine (оставляем как было) ---
    constexpr uint64_t Engine_GameViewport = 0x810;
    constexpr uint64_t Viewport_World = 0x78;
    constexpr uint64_t LocalPlayer_PC = 0x30;

    // --- Мир ---
    constexpr uint64_t World_PersistentLevel = 0x30;
    constexpr uint64_t Level_Actors = 0xA0;
    constexpr uint64_t Level_ActorCount = 0xA8;

    // --- Актор и персонаж (оставляем, они не менялись) ---
    constexpr uint64_t Actor_RootComponent = 0x208;
    constexpr uint64_t Scene_RelativeLocation = 0x1E4;
    constexpr uint64_t Char_Health = 0xE60;
    constexpr uint64_t Char_bDead = 0xE7C;
    constexpr uint64_t Char_PlayerKey = 0x980;

// --- Оффсеты для камеры (Аимбот) ---
constexpr uint64_t PC_PlayerCameraManager = 0x448; // Может отличаться для 4.6.0!
constexpr uint64_t PCM_Cache = 0x1A30;
constexpr uint64_t Cache_POV = 0x10;
constexpr uint64_t POV_Location = 0x10;
constexpr uint64_t POV_Rotation = 0x1C;
constexpr uint64_t POV_FOV = 0x24;
constexpr uint64_t PC_AcknowledgedPawn = 0x450;

// Если в sdk.cpp используется LocalPlayerPtr (а не LocalPlayer_PC)
constexpr uint64_t LocalPlayerPtr = 0x0; // Заглушка, если выдает ошибку
}

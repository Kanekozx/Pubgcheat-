#pragma once
#include <cstdint>

using ptr = uintptr_t;

namespace Off {

// --- глобалы (адреса от base libUE4.so) ---
constexpr ptr GEngine        = 0xF621D90;   // из твоего main.cpp — рабочий
constexpr ptr LocalPlayerPtr = 0xF5FBFF0;   // то, что в main.cpp было 0xF5FBFF0

// fallback (json 4.6.0, если GEngine-путь сломается)
constexpr ptr GWorld_static  = 0xF0DAA54;
constexpr ptr GNames         = 0xF08F830;
constexpr ptr GObjects       = 0xF360880;

// --- путь через GEngine ---
constexpr uint64_t Engine_GameViewport = 0x810;
constexpr uint64_t Viewport_World      = 0x78;
constexpr uint64_t LocalPlayer_PC      = 0x30;

// --- мир ---
constexpr uint64_t World_PersistentLevel = 0x30;
constexpr uint64_t Level_Actors          = 0xA0;
constexpr uint64_t Level_ActorCount      = 0xA8;

// --- актор ---
constexpr uint64_t Actor_RootComponent   = 0x208;
constexpr uint64_t Scene_RelativeLocation = 0x1E4;

// --- PC / камера ---
constexpr uint64_t PC_PlayerCameraManager = 0x548;
constexpr uint64_t PC_AcknowledgedPawn    = 0x528;
constexpr uint64_t PCM_Cache              = 0x520;
constexpr uint64_t Cache_POV              = 0x10;
constexpr uint64_t POV_Location           = 0x0;
constexpr uint64_t POV_Rotation           = 0x18;
constexpr uint64_t POV_FOV                = 0x24;

// --- персонаж ---
constexpr uint64_t Char_Health    = 0xE60;
constexpr uint64_t Char_bDead     = 0xE7C;
constexpr uint64_t Char_PlayerKey = 0x980;

} // namespace Off

#pragma once
#include "common.hpp"
#include "mem.hpp"     
#include "module.hpp"   
#include <string>

struct FVector  { float x, y, z; };
struct FRotator { float pitch, yaw, roll; };

struct FMinimalViewInfo {
    FVector  Location;
    u8       pad_0C[0xC];
    FRotator Rotation;
    u8       pad_24[0xC];
    float    FOV;
};

struct UEOffsets {
    ptr  GNames        = 0xF08F830;
    ptr  GObjects      = 0xF360880;
    ptr  Engine_LocalPlayer = 0xF0DA8EC;

    u32 UWorld_PersistentLevel        = 0x30;
    u32 UWorld_OwningGameInstance     = 0x470;
    u32 ULevel_Actors                 = 0xA0;
    u32 ULevel_ActorCount             = 0xA8;
    u32 AActor_RootComponent          = 0x208;
    u32 AActor_bHidden                = 0xE8;
    u32 USceneComponent_RelativeLocation = 0x1E4;
    u32 APlayerController_ControlRotation = 0x4E0;
    u32 APlayerController_PlayerCameraManager = 0x548;
    u32 APlayerController_AcknowledgedPawn = 0x528;
    u32 APlayerCameraManager_Cache    = 0x520;
    u32 CameraCache_POV               = 0x10;
    u32 MinimalViewInfo_Location      = 0x00;
    u32 MinimalViewInfo_Rotation      = 0x18;
    u32 MinimalViewInfo_FOV           = 0x24;
    u32 STExtraCharacter_Health       = 0xE60;
    u32 STExtraCharacter_bDead        = 0xE7C;
    u32 UAECharacter_PlayerName       = 0x960;
    u32 Pawn_Controller               = 0x4E8;
    u32 Controller_PlayerState        = 0x4D0;
};

// Проверка, является ли объект живым игроком/ботом
inline bool is_valid_player(const Proc& mem, const Module& ue4, const UEOffsets& off, ptr actor) {
    if (!actor) return false;
    
    // 1. Проверяем здоровье
    float health = mem.rd<float>(actor + off.STExtraCharacter_Health);
    if (health <= 0.0f || health > 200.0f) return false;

    // 2. Проверяем имя класса (чтобы не рисовать деревья)
    ptr class_ptr = mem.rd<ptr>(actor + 0x10);
    if (!class_ptr) return false;
    u32 name_idx = mem.rd<u32>(class_ptr + 0x18);
    if (name_idx == 0) return false;

    ptr gnames = ue4.base + off.GNames;
    ptr gnames_ptr = mem.rd<ptr>(gnames);
    if (!gnames_ptr) return false;

    ptr chunk = mem.rd<ptr>(gnames_ptr + 0x10 + (name_idx >> 16) * 8);
    if (!chunk) return false;
    ptr entry = mem.rd<ptr>(chunk + (name_idx & 0xFFFF) * 8);
    if (!entry) return false;

    u16 header = mem.rd<u16>(entry);
    u32 len = (header >> 6) & 0x3FF;
    if (len == 0 || len > 64) return false;

    char name[64] = {0};
    mem.read(entry + 2, name, len);

    if (strstr(name, "PlayerCharacter") || strstr(name, "STExtraCharacter") || strstr(name, "Character")) {
        return true;
    }
    return false;
}

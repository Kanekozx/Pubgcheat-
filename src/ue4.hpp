#pragma once
#include "common.hpp"
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
    // Глобальные указатели (GWorld и GEngine мы не используем, они зашифрованы)
    ptr  GNames        = 0xF08F830;  // Из твоего файла GName_Offset
    ptr  GObjects      = 0xF360880;  // Из твоего файла GUObject_Offset
    ptr  Engine_LocalPlayer = 0xF0DA8EC;

    // Оффсеты структур (из твоего 偏移量.json)
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

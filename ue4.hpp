#pragma once
#include "common.hpp"
#include <string>

struct FVector  { float x, y, z; };
struct FRotator { float pitch, yaw, roll; };

struct FMinimalViewInfo {
    FVector  Location;     // 0x00
    u8       pad_0C[0xC];  // 0x0C
    FRotator Rotation;     // 0x18
    u8       pad_24[0xC];  // 0x24 (Wait, FOV is at 0x24, so padding is actually up to 0x24)
    float    FOV;          // 0x24
    // ...
};

struct FCameraCacheEntry {
    float TimeStamp;       // 0x00
    u8    pad[0xC];        // 0x04
    FMinimalViewInfo POV;  // 0x10
};

// FString for reading Player Names
struct FString {
    wchar_t* Data;
    int32_t  Count;
    int32_t  Max;
};

struct UEOffsets {
    // Globals (Relative to libUE4.so base)
    ptr  GWorld        = 0xF0DAA54;
    ptr  GNames        = 0xF08F830;
    ptr  GObjects      = 0xF360880;
    ptr  Engine_LocalPlayer = 0xF0DA8EC;

    // UWorld
    u32 UWorld_PersistentLevel        = 0x30;
    u32 UWorld_OwningGameInstance     = 0x470;
    // ULevel
    u32 ULevel_Actors                 = 0xA0;
    u32 ULevel_ActorCount             = 0xA8;
    // AActor
    u32 AActor_RootComponent          = 0x208;
    u32 AActor_bHidden                = 0xE8;
    // USceneComponent
    u32 USceneComponent_RelativeLocation = 0x1E4;
    // APlayerController
    u32 APlayerController_ControlRotation = 0x4E0;
    u32 APlayerController_PlayerCameraManager = 0x548;
    u32 APlayerController_AcknowledgedPawn = 0x528;
    // APlayerCameraManager
    u32 APlayerCameraManager_Cache    = 0x520;
    // FCameraCacheEntry → POV
    u32 CameraCache_POV               = 0x10;
    // FMinimalViewInfo
    u32 MinimalViewInfo_Location      = 0x00;
    u32 MinimalViewInfo_Rotation      = 0x18;
    u32 MinimalViewInfo_FOV           = 0x24;
    // Character / PlayerState
    u32 STExtraCharacter_Health       = 0xE60;
    u32 STExtraCharacter_bDead        = 0xE7C;
    u32 UAECharacter_PlayerName       = 0x960;
    u32 Pawn_Controller               = 0x4E8;
    u32 Controller_PlayerState        = 0x4D0;
};
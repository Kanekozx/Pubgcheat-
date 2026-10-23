#include "offsets.hpp"
#include "common.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

static ptr parse_hex(const std::string& s) {
    if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        return (ptr)std::stoull(s.substr(2), nullptr, 16);
    return (ptr)std::stoull(s, nullptr, 10);
}

UEOffsets load_offsets(const std::string& path) {
    UEOffsets o;
    std::ifstream f(path);
    if (!f) { LOG("offsets: %s not found, using defaults", path.c_str()); return o; }

    std::stringstream ss; ss << f.rdbuf();
    std::string s = ss.str();

    auto get = [&](const char* key, auto& dst) {
        std::string needle = std::string("\"") + key + "\"";
        auto p = s.find(needle);
        if (p == std::string::npos) return;
        auto colon = s.find(':', p);
        auto comma = s.find_first_of(",}\n", colon);
        std::string val = s.substr(colon + 1, comma - colon - 1);
        while (!val.empty() && (val.front()==' '||val.front()=='\t')) val.erase(0,1);
        while (!val.empty() && (val.back()==' '||val.back()=='\t')) val.pop_back();
        try { dst = parse_hex(val); } catch (...) {}
    };

    get("GWorld", o.GWorld);
    get("GNames", o.GNames);
    get("GObjects", o.GObjects);
    get("Engine_LocalPlayer", o.Engine_LocalPlayer);
    get("UWorld_PersistentLevel", o.UWorld_PersistentLevel);
    get("UWorld_OwningGameInstance", o.UWorld_OwningGameInstance);
    get("ULevel_Actors", o.ULevel_Actors);
    get("ULevel_ActorCount", o.ULevel_ActorCount);
    get("AActor_RootComponent", o.AActor_RootComponent);
    get("AActor_bHidden", o.AActor_bHidden);
    get("USceneComponent_RelativeLocation", o.USceneComponent_RelativeLocation);
    get("APlayerController_ControlRotation", o.APlayerController_ControlRotation);
    get("APlayerController_PlayerCameraManager", o.APlayerController_PlayerCameraManager);
    get("APlayerController_AcknowledgedPawn", o.APlayerController_AcknowledgedPawn);
    get("APlayerCameraManager_Cache", o.APlayerCameraManager_Cache);
    get("CameraCache_POV", o.CameraCache_POV);
    get("MinimalViewInfo_Location", o.MinimalViewInfo_Location);
    get("MinimalViewInfo_Rotation", o.MinimalViewInfo_Rotation);
    get("MinimalViewInfo_FOV", o.MinimalViewInfo_FOV);
    get("STExtraCharacter_Health", o.STExtraCharacter_Health);
    get("STExtraCharacter_bDead", o.STExtraCharacter_bDead);
    get("UAECharacter_PlayerName", o.UAECharacter_PlayerName);
    get("Pawn_Controller", o.Pawn_Controller);
    get("Controller_PlayerState", o.Controller_PlayerState);

    LOG("offsets loaded from %s", path.c_str());
    return o;
}
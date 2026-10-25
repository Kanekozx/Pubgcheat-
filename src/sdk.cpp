#include "sdk.h"
#include "offsets.h"
#include "memory.h"

#include <dlfcn.h>
#include <link.h>
#include <cstring>
#include <cmath>

namespace SDK {

static ptr g_base = 0;

static int cb(dl_phdr_info* info, size_t, void*) {
    if (!info->dlpi_name || !*info->dlpi_name) return 0;
    if (strstr(info->dlpi_name, "libUE4.so") || strstr(info->dlpi_name, "libUE4Game.so")) {
        g_base = (ptr)info->dlpi_addr;
        return 1;
    }
    return 0;
}

bool init() {
    g_base = 0;
    dl_iterate_phdr(cb, nullptr);
    if (g_base) LOG("libUE4 base = 0x%lx", (unsigned long)g_base);
    return g_base != 0;
}

ptr base() { return g_base; }

static ptr getWorld() {
    ptr gengine = Mem::rdPtr(g_base + Off::GEngine);
    if (!gengine) return 0;
    ptr viewport = Mem::rdPtr(gengine + Off::Engine_GameViewport);
    if (!viewport) return 0;
    return Mem::rdPtr(viewport + Off::Viewport_World);
}

static ptr getLocalPC() {
    ptr lp = Mem::rdPtr(g_base + Off::LocalPlayerPtr);
    if (!lp) return 0;
    return Mem::rdPtr(lp + Off::LocalPlayer_PC);
}

bool getCamera(FMinimalViewInfo& pov) {
    ptr pc = getLocalPC();
    if (!pc) return false;
    ptr pcm = Mem::rdPtr(pc + Off::PC_PlayerCameraManager);
    if (!pcm) return false;

    ptr cache = pcm + Off::PCM_Cache + Off::Cache_POV;
    Mem::read(cache + Off::POV_Location, pov.Location);
    Mem::rd(cache + Off::POV_Rotation, pov.Rotation);
    Mem::rd(cache + Off::POV_FOV,      pov.FOV);
    return pov.FOV > 1.f && pov.FOV < 179.f;
}

ptr getLocalPawn() {
    ptr pc = getLocalPC();
    if (!pc) return 0;
    return Mem::rdPtr(pc + Off::PC_AcknowledgedPawn);
}

bool isValidPlayer(ptr actor) {
    if (!actor) return false;
    uint32_t key = Mem::rd<uint32_t>(actor + Off::Char_PlayerKey, 0);
    if (!key) return false;
    float hp = Mem::rd<float>(actor + Off::Char_Health, 0.f);
    if (hp <= 0.f || hp > 200.f) return false;
    bool dead = Mem::rd<bool>(actor + Off::Char_bDead, true);
    if (dead) return false;
    return true;
}

bool collectPlayers(std::vector<PlayerInfo>& out, int maxCount) {
    out.clear();
    FMinimalViewInfo pov{};
    if (!getCamera(pov)) return false;

    ptr world = getWorld();
    if (!world) return false;
    ptr level = Mem::rdPtr(world + Off::World_PersistentLevel);
    if (!level) return false;
    ptr actors = Mem::rdPtr(level + Off::Level_Actors);
    uint32_t count = Mem::rd<uint32_t>(level + Off::Level_ActorCount, 0);
    if (!actors || count == 0 || count > 100000) return false;

    ptr me = getLocalPawn();

    for (uint32_t i = 0; i < count && (int)out.size() < maxCount; ++i) {
        ptr actor = Mem::rdPtr(actors + i * 8);
        if (!actor || actor == me) continue;
        if (!isValidPlayer(actor)) continue;

        ptr root = Mem::rdPtr(actor + Off::Actor_RootComponent);
        if (!root) continue;
        FVector loc{};
        if (!Mem::rd(root + Off::Scene_RelativeLocation, loc)) continue;

        float dx = loc.X - pov.Location.X;
        float dy = loc.Y - pov.Location.Y;
        float dz = loc.Z - pov.Location.Z;
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz) / 100.f;

        float hp = Mem::rd<float>(actor + Off::Char_Health, 0.f);
        out.push_back({ actor, loc, hp, dist });
    }
    return true;
}

bool worldToScreen(const FVector& w, const FMinimalViewInfo& pov,
                   int sw, int sh, float& sx, float& sy)
{
    const float D2R = 3.14159265f / 180.f;
    FVector d = { w.X - pov.Location.X, w.Y - pov.Location.Y, w.Z - pov.Location.Z };
    float cy = cosf(pov.Rotation.Yaw   * D2R), sy_ = sinf(pov.Rotation.Yaw   * D2R);
    float cp = cosf(pov.Rotation.Pitch * D2R), sp  = sinf(pov.Rotation.Pitch * D2R);

    float fwd   = d.X * cp * cy + d.Y * cp * sy_ - d.Z * sp;
    float right = -d.X * sy_ + d.Y * cy;
    float up    = d.X * sp * cy + d.Y * sp * sy_ + d.Z * cp;
    if (fwd < 1.f) return false;

    float f = (sw * 0.5f) / tanf(pov.FOV * 0.5f * D2R);
    sx = sw * 0.5f + (right / fwd) * f;
    sy = sh * 0.5f - (up    / fwd) * f;
    return true;
}

} // namespace SDK

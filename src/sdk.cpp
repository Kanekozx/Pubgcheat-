#include "sdk.h"
#include "offsets.h"
#include <android/log.h>
#include <cmath>
#include <cstring>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "UE4", __VA_ARGS__)

uintptr_t base_addr = 0;

// Получаем адрес библиотеки libUE4.so (через /proc/self/maps)
void* getModuleBase(const char* name) {
    if (base_addr != 0) return (void*)base_addr;
    
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return nullptr;
    
    char line[512];
    uintptr_t addr = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, name)) {
            sscanf(line, "%lx-%*lx", &addr);
            break;
        }
    }
    fclose(fp);
    base_addr = addr;
    return (void*)base_addr;
}

bool getCamera(FMinimalViewInfo& pov) {
    void* base = getModuleBase("libUE4.so");
    if (!base) return false;

    // Читаем GWorld (из твоих оффсетов)
    uintptr_t uworld = *(uintptr_t*)((uintptr_t)base + Off::GWorld);
    if (!uworld) return false;

    // Путь: UWorld -> OwningGameInstance -> LocalPlayers -> PlayerController -> CameraManager -> POV
    uintptr_t gameInstance = *(uintptr_t*)(uworld + 0x1A8); 
    if (!gameInstance) return false;
    
    uintptr_t localPlayers = *(uintptr_t*)(gameInstance + 0x38);
    if (!localPlayers) return false;
    
    uintptr_t localPlayer = *(uintptr_t*)(localPlayers);
    if (!localPlayer) return false;
    
    uintptr_t playerController = *(uintptr_t*)(localPlayer + 0x30);
    if (!playerController) return false;
    
    uintptr_t cameraManager = *(uintptr_t*)(playerController + 0x478); 
    if (!cameraManager) return false;

    // Читаем POV (Location, Rotation, FOV)
    pov = *(FMinimalViewInfo*)(cameraManager + 0x1470);
    return true;
}

std::vector<PlayerInfo> GetPlayers() {
    std::vector<PlayerInfo> players;
    
    void* base = getModuleBase("libUE4.so");
    if (!base) return players;

    uintptr_t uworld = *(uintptr_t*)((uintptr_t)base + Off::GWorld);
    if (!uworld) return players;

    // Находим камеру, чтобы считать дистанцию
    FMinimalViewInfo cam;
    if (!getCamera(cam)) return players;

    uintptr_t persistentLevel = *(uintptr_t*)(uworld + Off::World_PersistentLevel);
    if (!persistentLevel) return players;

    uintptr_t actors = *(uintptr_t*)(persistentLevel + Off::Level_Actors);
    int actorCount = *(int*)(persistentLevel + Off::Level_ActorCount);

    if (!actors || actorCount <= 0 || actorCount > 10000) return players;

    for (int i = 0; i < actorCount; i++) {
        uintptr_t actor = *(uintptr_t*)(actors + (i * 8));
        if (!actor) continue;

        // Проверяем, что это игрок (нужно проверять по имени класса или здоровью)
        // В реальном чите тут проверка GNames, но для теста просто читаем координаты
        uintptr_t rootComponent = *(uintptr_t*)(actor + Off::Actor_RootComponent);
        if (!rootComponent) continue;

        FVector loc = *(FVector*)(rootComponent + Off::Scene_RelativeLocation);
        
        // Считаем дистанцию до игрока
        FVector delta = loc - cam.Location;
        float dist = std::sqrt(delta.X * delta.X + delta.Y * delta.Y + delta.Z * delta.Z) / 100.0f; // В метры

        // Игнорируем далеких и себя
        if (dist > 0.1f && dist < 500.0f) {
            PlayerInfo p;
            p.Actor = actor;
            p.Location = loc;
            p.Distance = dist;
            players.push_back(p);
        }
    }
    return players;
}

// Перевод 3D координат в 2D экран
bool WorldToScreen(FVector WorldLocation, FMinimalViewInfo cam, FVector& ScreenLocation, int ScreenW, int ScreenH) {
    // Вектор от камеры до цели
    FVector delta = WorldLocation - cam.Location;

    // Математика UE4 (перевод в радианы и проекция)
    float radPitch = cam.Rotation.Pitch * (3.14159265358979323846f / 180.0f);
    float radYaw = cam.Rotation.Yaw * (3.14159265358979323846f / 180.0f);
    float radRoll = cam.Rotation.Roll * (3.14159265358979323846f / 180.0f);

    float SP = sinf(radPitch);
    float CP = cosf(radPitch);
    float SY = sinf(radYaw);
    float CY = cosf(radYaw);
    float SR = sinf(radRoll);
    float CR = cosf(radRoll);

    float x = delta.X * (CY * CP) + delta.Y * (SY * CP) + delta.Z * (-SP);
    float y = delta.X * (SY * CR - CY * SR * SP) + delta.Y * (-CY * CR - SY * SR * SP) + delta.Z * (-SR * CP);
    float z = delta.X * (CY * SR + SY * CR * SP) + delta.Y * (SY * SR - CY * CR * SP) + delta.Z * (CR * CP);

    if (z < 1.0f) return false; // Позади камеры

    float fovRad = cam.FOV * (3.14159265358979323846f / 180.0f);
    float tanHalfFov = tanf(fovRad / 2.0f);

    ScreenLocation.X = (ScreenW / 2.0f) * (1.0f + x / (z * tanHalfFov * (ScreenW / (float)ScreenH)));
    ScreenLocation.Y = (ScreenH / 2.0f) * (1.0f - y / (z * tanHalfFov));
    ScreenLocation.Z = z;
    
    return true;
}

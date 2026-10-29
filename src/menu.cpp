#include <EGL/egl.h>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "sdk.h"

bool bDrawESP = true;
bool bDrawMenu = true;
float g_LoadProgress = 0.1f;
bool g_IsReady = false;

void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.Fonts->AddFontDefault();
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    io.MouseDrawCursor = false; 
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    io.FontGlobalScale = 1.5f;
    ImGui_ImplOpenGL3_Init("#version 300 es");
}

void ImGuiBeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void DrawESP() {
    if (!bDrawESP || !g_IsReady) return;
    
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    
    FMinimalViewInfo camera;
    if (!getCamera(camera)) return; // Если камеру не нашли, ничего не рисуем

    std::vector<PlayerInfo> players = GetPlayers();

    for (auto& p : players) {
        FVector screenPos;
        // Переводим 3D-координаты врага в 2D-экран
        if (WorldToScreen(p.Location, camera, screenPos, (int)io.DisplaySize.x, (int)io.DisplaySize.y)) {
            
            // Рисуем зеленый квадрат вокруг игрока
            // (Ширина и высота зависят от дистанции)
            float boxWidth = 2000.0f / screenPos.Z;
            float boxHeight = 4000.0f / screenPos.Z;
            
            ImVec2 p1(screenPos.X - boxWidth / 2, screenPos.Y - boxHeight / 2);
            ImVec2 p2(screenPos.X + boxWidth / 2, screenPos.Y + boxHeight / 2);
            
            drawList->AddRect(p1, p2, IM_COL32(0, 255, 0, 255), 0.0f, 0, 2.0f);
            
            // Рисуем текст с дистанцией
            char distText[32];
            sprintf(distText, "%.1fm", p.Distance);
            drawList->AddText(ImVec2(p1.x, p1.y - 25), IM_COL32(255, 255, 255, 255), distText);
        }
    }
}

void DrawMenu() {
    if (!bDrawMenu) return;
    ImGui::SetNextWindowPos(ImVec2(20, 50), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    
    if (!g_IsReady) {
        ImGui::Begin("System Loading", nullptr, flags | ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("Initializing Cheat Engine...");
        ImGui::Spacing();
        ImGui::ProgressBar(g_LoadProgress, ImVec2(-1, 0));
        ImGui::End();
        return;
    }

    ImGui::Begin("PUBG CHEAT", &bDrawMenu, flags);
    ImGui::Text("Menu is active!");
    ImGui::Spacing();
    ImGui::Checkbox("Enable ESP", &bDrawESP);
    ImGui::End();
}

void ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

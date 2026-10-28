#include <EGL/egl.h>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "sdk.h" // <--- Подключаем наш SDK

bool bDrawESP = true;
bool bDrawMenu = true;
bool bDrawTestBox = true; // Для теста

void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.Fonts->AddFontDefault();
    io.MouseDrawCursor = true; 
    ImGui_ImplOpenGL3_Init("#version 300 es");
}

void ImGuiBeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void DrawESP() {
    if (!bDrawESP) return;
    
    ImGuiIO& io = ImGui::GetIO();
    
    // === ТЕСТОВЫЙ КВАДРАТ В ЦЕНТРЕ ЭКРАНА ===
    if (bDrawTestBox) {
        ImVec2 p1(io.DisplaySize.x / 2 - 50, io.DisplaySize.y / 2 - 100);
        ImVec2 p2(io.DisplaySize.x / 2 + 50, io.DisplaySize.y / 2 + 100);
        ImGui::GetBackgroundDrawList()->AddRect(p1, p2, IM_COL32(0, 255, 0, 255), 0.0f, 0, 2.0f);
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(p1.x, p1.y - 20), IM_COL32(255, 255, 255, 255), "TEST PLAYER");
    }

    // === НАСТОЯЩИЙ ESP (пока не работает без матрицы) ===
    // std::vector<uintptr_t> players = GetPlayers();
    // for (auto player : players) {
    //     FVector worldLoc = ...; // Читаем координаты
    //     FVector screenLoc;
    //     if (WorldToScreen(worldLoc, screenLoc, io.DisplaySize.x, io.DisplaySize.y)) {
    //         ImGui::GetBackgroundDrawList()->AddRect(...);
    //     }
    // }
}

void DrawMenu() {
    if (bDrawMenu) {
        ImGui::Begin("PUBG CHEAT", &bDrawMenu);
        ImGui::Checkbox("Enable ESP", &bDrawESP);
        ImGui::Checkbox("Draw Test Box", &bDrawTestBox);
        ImGui::Text("My Custom Cheat is Working!");
        ImGui::End();
    }
}

void ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

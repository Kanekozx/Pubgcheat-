#include <EGL/egl.h>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "sdk.h" // Наш SDK

bool bDrawESP = true;
bool bDrawMenu = false; // МЕНЮ ВЫКЛЮЧЕНО

void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.Fonts->AddFontDefault();
    
    // Увеличиваем шрифт для ESP
    io.FontGlobalScale = 1.2f;

    ImGui_ImplOpenGL3_Init("#version 300 es");
}

void ImGuiBeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void DrawESP() {
    if (!bDrawESP) return;
    
    ImGuiIO& io = ImGui::GetIO();
    
    // ТЕСТОВЫЙ ЦИКЛ: Рисуем 3 бокса в разных местах экрана
    // (Потом мы заменим это на реальные координаты из sdk.cpp)
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    
    float centerX = io.DisplaySize.x / 2.0f;
    float centerY = io.DisplaySize.y / 2.0f;

    // Игрок 1 (Слева)
    drawList->AddRect(ImVec2(centerX - 400, centerY - 200), ImVec2(centerX - 300, centerY + 100), IM_COL32(255, 0, 0, 255), 0.0f, 0, 3.0f);
    drawList->AddText(ImVec2(centerX - 400, centerY - 230), IM_COL32(255, 255, 255, 255), "Enemy [50m]");

    // Игрок 2 (По центру)
    drawList->AddRect(ImVec2(centerX - 50, centerY - 150), ImVec2(centerX + 50, centerY + 150), IM_COL32(255, 0, 0, 255), 0.0f, 0, 3.0f);
    drawList->AddText(ImVec2(centerX - 50, centerY - 180), IM_COL32(255, 255, 255, 255), "Enemy [10m]");

    // Игрок 3 (Справа)
    drawList->AddRect(ImVec2(centerX + 300, centerY - 100), ImVec2(centerX + 400, centerY + 200), IM_COL32(255, 0, 0, 255), 0.0f, 0, 3.0f);
    drawList->AddText(ImVec2(centerX + 300, centerY - 130), IM_COL32(255, 255, 255, 255), "Enemy [80m]");
}

void DrawMenu() {
    // Меню отключено, функция пустая
}

void ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

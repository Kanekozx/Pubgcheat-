#include <EGL/egl.h>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "sdk.h"

bool bDrawESP = true;
bool bDrawMenu = true;
bool bDrawTestBox = true; // Для проверки

void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    // Устанавливаем размер экрана
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.Fonts->AddFontDefault();
    
    // Включаем курсор, чтобы видеть точку нажатия
    io.MouseDrawCursor = true; 

    // ДЕЛАЕМ ИНТЕРФЕЙС БОЛЬШИМ (чтобы пальцем попадать)
    ImGui::GetStyle().ScaleAllSizes(3.0f); 
    io.FontGlobalScale = 2.0f;

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
        ImVec2 p1(io.DisplaySize.x / 2 - 100, io.DisplaySize.y / 2 - 200);
        ImVec2 p2(io.DisplaySize.x / 2 + 100, io.DisplaySize.y / 2 + 200);
        
        // Рисуем зеленый квадрат
        ImGui::GetBackgroundDrawList()->AddRect(p1, p2, IM_COL32(0, 255, 0, 255), 0.0f, 0, 4.0f);
        // Рисуем текст над квадратом
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(p1.x, p1.y - 40), IM_COL32(255, 255, 255, 255), "TEST PLAYER");
    }
}

void DrawMenu() {
    if (bDrawMenu) {
        // Задаем начальный размер окна (большой)
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("PUBG CHEAT", &bDrawMenu);
        
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Welcome to my cheat!");
        ImGui::Spacing();
        
        // Чекбоксы (кнопки)
        ImGui::Checkbox("Enable ESP", &bDrawESP);
        ImGui::Checkbox("Draw Test Box", &bDrawTestBox);
        
        ImGui::Spacing();
        
        // Тестовая кнопка (чтобы проверить нажатия)
        if (ImGui::Button("Click Me!")) {
            __android_log_print(ANDROID_LOG_INFO, "UE4", "Button clicked!");
        }

        ImGui::End();
    }
}

void ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

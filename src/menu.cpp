#include <EGL/egl.h>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

bool bDrawESP = true;
bool bDrawMenu = true;

void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.Fonts->AddFontDefault();
    
    // Включаем тач и рисуем курсор
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    io.MouseDrawCursor = true;
    
    // Делаем шрифт и кнопки большими
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    io.FontGlobalScale = 1.5f;

    ImGui_ImplOpenGL3_Init("#version 300 es");
}

void ImGuiBeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void DrawESP() {
    if (!bDrawESP) return;
    
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    
    float centerX = io.DisplaySize.x / 2.0f;
    float centerY = io.DisplaySize.y / 2.0f;

    // ТЕСТОВЫЕ БОКСЫ (Auto-ESP)
    drawList->AddRect(ImVec2(centerX - 400, centerY - 200), ImVec2(centerX - 300, centerY + 100), IM_COL32(255, 0, 0, 255), 0.0f, 0, 3.0f);
    drawList->AddRect(ImVec2(centerX - 50, centerY - 150), ImVec2(centerX + 50, centerY + 150), IM_COL32(255, 0, 0, 255), 0.0f, 0, 3.0f);
    drawList->AddRect(ImVec2(centerX + 300, centerY - 100), ImVec2(centerX + 400, centerY + 200), IM_COL32(255, 0, 0, 255), 0.0f, 0, 3.0f);
}

void DrawMenu() {
    if (!bDrawMenu) return;

    // ЖЕСТКО в левый верхний угол
    ImGui::SetNextWindowPos(ImVec2(20, 50), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Always);
    
    // Запрещаем двигать и изменять размер
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    
    ImGui::Begin("PUBG CHEAT", &bDrawMenu, flags);
    
    ImGui::Text("Menu is active!");
    ImGui::Spacing();
    ImGui::Checkbox("Enable ESP", &bDrawESP);
    ImGui::Spacing();
    
    if (ImGui::Button("Click Me!")) {
        __android_log_print(ANDROID_LOG_INFO, "UE4", "Button clicked!");
    }

    ImGui::End();
}

void ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

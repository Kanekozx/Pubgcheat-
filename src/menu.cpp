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
    
    // Увеличиваем шрифт, чтобы на телефоне было видно
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

    // ЖЕСТКО ФИКСИРУЕМ ПОЗИЦИЮ (20 пикселей от левого и верхнего края)
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
    // ЖЕСТКО ЗАДАЕМ РАЗМЕР ОКНА
    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_Always);
    
    // Запрещаем двигать, менять размер и сворачивать окно
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    
    ImGui::Begin("PUBG CHEAT", &bDrawMenu, flags);
    
    ImGui::Text("Menu is active!");
    ImGui::Spacing();
    
    // Галочки (они будут работать, если мы починим тач, но отрисовываться будут всегда)
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

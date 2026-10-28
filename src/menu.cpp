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
    
    ImGui_ImplOpenGL3_Init("#version 300 es");
    __android_log_print(ANDROID_LOG_INFO, "PubgCheat", "ImGui Init OK");
}

// Новая функция для начала кадра
void ImGuiBeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void DrawESP() {
    if (!bDrawESP) return;
    // Рисуем тестовую красную линию из левого верхнего угла в центр экрана
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(100, 100), ImVec2(1000, 500), IM_COL32(255, 0, 0, 255), 5.0f);
}

void DrawMenu() {
    if (bDrawMenu) {
        ImGui::Begin("PUBG CHEAT", &bDrawMenu);
        ImGui::Checkbox("Enable ESP (Red Line)", &bDrawESP);
        ImGui::Text("My Custom Cheat is Working!");
        ImGui::End();
    }
}

void ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

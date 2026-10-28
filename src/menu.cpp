#include <EGL/egl.h>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

bool bDrawESP = true;
bool bDrawMenu = true;

void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Устанавливаем размер экрана, чтобы меню не было размером 0x0
    io.DisplaySize = ImVec2((float)w, (float)h);
    
    ImGui_ImplOpenGL3_Init("#version 300 es");
    __android_log_print(ANDROID_LOG_INFO, "PubgCheat", "ImGui Initialized: %dx%d", w, h);
}

void DrawESP() {
    if (!bDrawESP) return;
    // Тут будет код рисования
}

void DrawMenu() {
    // ЭТИ ДВЕ СТРОКИ ОБЯЗАТЕЛЬНЫ ДЛЯ РАБОТЫ!
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (bDrawMenu) {
        ImGui::Begin("PUBG CHEAT", &bDrawMenu);
        ImGui::Checkbox("Enable ESP", &bDrawESP);
        ImGui::Text("My Custom Cheat is Working!");
        ImGui::End();
    }
}

void ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

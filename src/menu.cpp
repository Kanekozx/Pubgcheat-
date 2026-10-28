#include <EGL/egl.h>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

// Здесь мы объявляем глобальные настройки чита
bool bDrawESP = true;
bool bDrawMenu = true;

// Функция инициализации ImGui (вызывается из module.cpp)
void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    ImGui_ImplOpenGL3_Init("#version 300 es");
    __android_log_print(ANDROID_LOG_INFO, "PubgCheat", "ImGui Initialized!");
}

// Отрисовка ESP (пока пустая заглушка)
void DrawESP() {
    if (!bDrawESP) return;
    // Тут будет код рисования линий, боксов и т.д.
}

// Отрисовка самого меню
void DrawMenu() {
    if (!bDrawMenu) return;
    
    ImGui::Begin("PUBG CHEAT", &bDrawMenu);
    ImGui::Checkbox("Enable ESP", &bDrawESP);
    ImGui::Text("Welcome to my cheat!");
    ImGui::End();
}

// Рендер кадра ImGui
void ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

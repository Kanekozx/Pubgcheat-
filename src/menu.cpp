#include <EGL/egl.h>
#include <android/log.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

bool bDrawESP = true;
bool bDrawMenu = true;
bool bDrawTestBox = true;

void ImGuiInit(EGLDisplay dpy, EGLSurface surf, int w, int h) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.Fonts->AddFontDefault();
    
    // Включаем поддержку тач-экрана
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen; 
    io.MouseDrawCursor = true; 
    
    // Делаем интерфейс больше
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
    
    // Тестовый квадрат
    if (bDrawTestBox) {
        ImVec2 p1(io.DisplaySize.x / 2 - 100, io.DisplaySize.y / 2 - 200);
        ImVec2 p2(io.DisplaySize.x / 2 + 100, io.DisplaySize.y / 2 + 200);
        
        ImGui::GetBackgroundDrawList()->AddRect(p1, p2, IM_COL32(0, 255, 0, 255), 0.0f, 0, 4.0f);
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(p1.x, p1.y - 30), IM_COL32(255, 255, 255, 255), "TEST PLAYER");
    }
}

void DrawMenu() {
    if (bDrawMenu) {
        // Большое окно
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("PUBG CHEAT", &bDrawMenu);
        
        ImGui::Text("Welcome to my cheat!");
        ImGui::Spacing();
        
        ImGui::Checkbox("Enable ESP", &bDrawESP);
        ImGui::Checkbox("Draw Test Box", &bDrawTestBox);
        
        ImGui::Spacing();
        
        if (ImGui::Button("Click Me!")) {
            __android_log_print(ANDROID_LOG_INFO, "UE4", "Button Clicked!");
        }

        ImGui::End();
    }
}

void ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

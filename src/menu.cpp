#include "menu.h"
#include "config.h"

#include <imgui.h>

void drawMenu(int, int) {
    if (!g_cfg.menuOpen) return;

    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(420, 420), ImGuiCond_FirstUseEver);
    ImGui::Begin("ESP", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Checkbox("Enable",     &g_cfg.enabled);
    ImGui::Checkbox("Box",        &g_cfg.box);
    ImGui::Checkbox("Health",     &g_cfg.health);
    ImGui::Checkbox("Distance",   &g_cfg.distance);
    ImGui::Checkbox("Snaplines",  &g_cfg.snaplines);
    ImGui::SliderFloat("Max dist", &g_cfg.maxDist, 100.f, 1500.f, "%.0f");
    ImGui::ColorEdit3("Box color", g_cfg.boxColor);

    ImGui::End();
}

#include "esp.h"
#include "sdk.h"
#include "config.h"

#include <imgui.h>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <vector>

void drawESP(int sw, int sh) {
    if (!g_cfg.enabled) return;

    std::vector<SDK::PlayerInfo> players;
    if (!SDK::collectPlayers(players)) return;

    FMinimalViewInfo pov{};
    if (!SDK::getCamera(pov)) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    dl->PushClipRectFullScreen();

    for (auto& p : players) {
        if (p.dist > g_cfg.maxDist) continue;

        FVector headW { p.world.X, p.world.Y, p.world.Z + 90.f };
        FVector feetW { p.world.X, p.world.Y, p.world.Z - 90.f };

        float hx, hy, fx, fy;
        if (!SDK::worldToScreen(headW, pov, sw, sh, hx, hy)) continue;
        if (!SDK::worldToScreen(feetW, pov, sw, sh, fx, fy)) continue;

        float h = std::fabs(fy - hy);
        float w = h * 0.55f;
        if (h < 6.f || h > 3000.f) continue;

        float x0 = hx - w * 0.5f, x1 = hx + w * 0.5f;
        float y0 = hy, y1 = fy;

        if (g_cfg.box) {
            ImVec4 col(g_cfg.boxColor[0], g_cfg.boxColor[1], g_cfg.boxColor[2], 1.f);
            dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1),
                        ImGui::GetColorU32(col), 0.f, 0, 1.5f);
        }

        if (g_cfg.health) {
            float hpFrac = std::clamp(p.hp / 100.f, 0.f, 1.f);
            dl->AddRectFilled(ImVec2(x0 - 5, y0), ImVec2(x0 - 2, y1),
                              IM_COL32(0, 0, 0, 180));
            dl->AddRectFilled(ImVec2(x0 - 5, y1 - (y1 - y0) * hpFrac),
                              ImVec2(x0 - 2, y1), IM_COL32(0, 255, 0, 255));
        }

        if (g_cfg.distance) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%dm", (int)p.dist);
            dl->AddText(ImVec2(hx, y0 - 16.f), IM_COL32(255, 255, 0, 255), buf);
        }

        if (g_cfg.snaplines) {
            dl->AddLine(ImVec2((float)sw * 0.5f, (float)sh),
                        ImVec2(hx, y1), IM_COL32(255, 255, 255, 120), 1.f);
        }
    }

    dl->PopClipRect();
}

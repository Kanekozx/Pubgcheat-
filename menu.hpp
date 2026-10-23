#pragma once
#include <atomic>

extern std::atomic<bool> g_esp_enabled;
extern std::atomic<bool> g_aim_enabled;
extern std::atomic<bool> g_aim_draw_fov;
extern std::atomic<float> g_aim_fov;
extern std::atomic<float> g_aim_smooth;

void start_web_menu();
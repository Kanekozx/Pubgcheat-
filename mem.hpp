#pragma once
#include <atomic>
#include <vector>
#include <string>
#include <mutex>

extern std::atomic<bool> g_esp_enabled;
extern std::atomic<bool> g_aim_enabled;
extern std::atomic<bool> g_aim_draw_fov;
extern std::atomic<float> g_aim_fov;
extern std::atomic<float> g_aim_smooth;

struct BoxData {
    float x, y, w, h;
    std::string text;
};

extern std::vector<BoxData> g_draw_list;
extern std::mutex g_draw_mutex;

void start_web_menu();

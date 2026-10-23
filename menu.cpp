#include "menu.hpp"
#include "common.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <thread>
#include <sstream>

std::atomic<bool> g_esp_enabled{true};
std::atomic<bool> g_aim_enabled{true};
std::atomic<bool> g_aim_draw_fov{true};
std::atomic<float> g_aim_fov{15.0f};
std::atomic<float> g_aim_smooth{4.0f};

std::vector<BoxData> g_draw_list;
std::mutex g_draw_mutex;

static const char* HTML_PAGE = R"html(
<!DOCTYPE html>
<html><head><meta name="viewport" content="width=device-width, initial-scale=1"></head>
<body style="background:#121212;color:#fff;font-family:sans-serif;padding:20px;">
    <h1>PUBG Cheat Menu</h1>
    <button onclick="fetch('/toggle_esp')">Toggle ESP</button>
    <p>Use the Android Overlay app to view ESP.</p>
</body></html>
)html";

void start_web_menu() {
    std::thread([]() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(8080);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            return;
        }
        listen(server_fd, 3);
        LOG("Web menu running at http://127.0.0.1:8080");

        while (true) {
            int new_socket = accept(server_fd, nullptr, nullptr);
            if (new_socket < 0) continue;

            char buffer[1024] = {0};
            read(new_socket, buffer, 1024);
            std::string req(buffer);

            std::string response;
            if (req.find("GET /toggle_esp") != std::string::npos) {
                g_esp_enabled = !g_esp_enabled;
                response = "HTTP/1.1 200 OK\r\n\r\n";
            } else if (req.find("GET /get_boxes") != std::string::npos) {
                std::string json = "[";
                std::lock_guard<std::mutex> lock(g_draw_mutex);
                for (size_t i = 0; i < g_draw_list.size(); ++i) {
                    auto& b = g_draw_list[i];
                    json += "{\"x\":" + std::to_string(b.x) + ",\"y\":" + std::to_string(b.y) + 
                            ",\"w\":" + std::to_string(b.w) + ",\"h\":" + std::to_string(b.h) + 
                            ",\"text\":\"" + b.text + "\"}";
                    if (i < g_draw_list.size() - 1) json += ",";
                }
                json += "]";
                response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + json;
            } else {
                response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
                response += HTML_PAGE;
            }
            send(new_socket, response.c_str(), response.length(), 0);
            close(new_socket);
        }
    }).detach();
}

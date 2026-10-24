#include "menu.hpp"
#include "common.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <thread>
#include <sstream>
#include <cstdio>

std::atomic<bool> g_esp_enabled{true};
std::atomic<bool> g_aim_enabled{true};
std::atomic<bool> g_aim_draw_fov{true};
std::atomic<float> g_aim_fov{15.0f};
std::atomic<float> g_aim_smooth{4.0f};

std::vector<BoxData> g_draw_list;
std::mutex g_draw_mutex;

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
            printf("[-] Web server bind failed\n");
            return;
        }
        listen(server_fd, 3);
        printf("[*] Web server running at http://127.0.0.1:8080\n");
        fflush(stdout);

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
                
                printf("[HTTP] get_boxes requested. Items in list: %zu\n", g_draw_list.size());
                fflush(stdout);

                char buf[256];
                for (size_t i = 0; i < g_draw_list.size(); ++i) {
                    auto& b = g_draw_list[i];
                    snprintf(buf, sizeof(buf), 
                        "{\"x\":%.1f,\"y\":%.1f,\"w\":%.1f,\"h\":%.1f,\"text\":\"%s\"}", 
                        b.x, b.y, b.w, b.h, b.text.c_str());
                    json += buf;
                    if (i < g_draw_list.size() - 1) json += ",";
                }
                json += "]";
                response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + json;
            } else {
                response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
                response += "<html><body>OK</body></html>";
            }
            send(new_socket, response.c_str(), response.length(), 0);
            close(new_socket);
        }
    }).detach();
}

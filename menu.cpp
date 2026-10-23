#include "menu.hpp"
#include "common.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <thread>

std::atomic<bool> g_esp_enabled{true};
std::atomic<bool> g_aim_enabled{true};
std::atomic<bool> g_aim_draw_fov{true};
std::atomic<float> g_aim_fov{15.0f};
std::atomic<float> g_aim_smooth{4.0f};

// Using a custom delimiter R"html( ... )html" to prevent compile errors
static const char* HTML_PAGE = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>PUBG Menu</title>
    <style>
        body { background: #121212; color: #fff; font-family: sans-serif; padding: 20px; }
        h1 { text-align: center; color: #00e5ff; }
        .card { background: #1e1e1e; padding: 15px; margin-bottom: 15px; border-radius: 8px; }
        .row { display: flex; justify-content: space-between; align-items: center; margin: 10px 0; }
        button { background: #00e5ff; border: none; padding: 10px 20px; border-radius: 5px; font-weight: bold; cursor: pointer;}
        .on { background: #4caf50; color: white; }
        .off { background: #f44336; color: white; }
    </style>
</head>
<body>
    <h1>PUBG Cheat Menu</h1>
    <div class="card">
        <div class="row">
            <span>ESP Enabled</span>
            <button id="esp_btn" onclick="toggle('esp')">...</button>
        </div>
        <div class="row">
            <span>Aimbot Enabled</span>
            <button id="aim_btn" onclick="toggle('aim')">...</button>
        </div>
        <div class="row">
            <span>Draw FOV Circle</span>
            <button id="fov_btn" onclick="toggle('fov')">...</button>
        </div>
    </div>
    <script>
        function toggle(feature) {
            fetch('/toggle_' + feature).then(() => updateState());
        }
        function updateState() {
            fetch('/state').then(r => r.json()).then(data => {
                document.getElementById('esp_btn').className = data.esp ? 'on' : 'off';
                document.getElementById('esp_btn').innerText = data.esp ? 'ON' : 'OFF';
                document.getElementById('aim_btn').className = data.aim ? 'on' : 'off';
                document.getElementById('aim_btn').innerText = data.aim ? 'ON' : 'OFF';
                document.getElementById('fov_btn').className = data.fov ? 'on' : 'off';
                document.getElementById('fov_btn').innerText = data.fov ? 'ON' : 'OFF';
            });
        }
        setInterval(updateState, 1000);
        updateState();
    </script>
</body>
</html>
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
            ERR("Web menu failed to bind port 8080");
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
            } else if (req.find("GET /toggle_aim") != std::string::npos) {
                g_aim_enabled = !g_aim_enabled;
                response = "HTTP/1.1 200 OK\r\n\r\n";
            } else if (req.find("GET /toggle_fov") != std::string::npos) {
                g_aim_draw_fov = !g_aim_draw_fov;
                response = "HTTP/1.1 200 OK\r\n\r\n";
            } else if (req.find("GET /state") != std::string::npos) {
                std::string json = "{\"esp\":" + std::to_string(g_esp_enabled.load()) + 
                                   ",\"aim\":" + std::to_string(g_aim_enabled.load()) + 
                                   ",\"fov\":" + std::to_string(g_aim_draw_fov.load()) + "}";
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
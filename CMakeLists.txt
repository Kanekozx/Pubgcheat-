cmake_minimum_required(VERSION 3.18)
project(pubg_internal CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем Dobby (хуки)
add_subdirectory(Dobby)
include_directories(Dobby/include)

# Подключаем ImGui (меню)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_android.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)

# Собираем нашу библиотеку
add_library(main SHARED src/main.cpp)
target_link_libraries(main imgui dobby android log EGL GLESv3)

target_compile_options(main PRIVATE -O2 -fvisibility=hidden)

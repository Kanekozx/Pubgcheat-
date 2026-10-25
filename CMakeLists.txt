cmake_minimum_required(VERSION 3.22)
project(pubgcheat CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O2 -s -Wno-unused-function")

# --- xhook через FetchContent ---
include(FetchContent)
FetchContent_Declare(
    xhook
    GIT_REPOSITORY https://github.com/iqiyi/xHook.git
    GIT_TAG        v1.2.2
)
FetchContent_MakeAvailable(xhook)

# --- ImGui (как было у тебя) ---
add_library(imgui STATIC
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    src/imgui
    src/imgui/backends
)

# --- Модуль ---
add_library(pubgcheat SHARED
    module.cpp
    src/sdk.cpp
    src/menu.cpp
)

target_include_directories(pubgcheat PRIVATE
    src
    ${CMAKE_SOURCE_DIR}
)

target_compile_definitions(pubgcheat PRIVATE PTRACE_MODE)

target_link_libraries(pubgcheat
    imgui
    xhook
    log
    android
    EGL
    GLESv3
    dl
)

# Только arm64
set_target_properties(pubgcheat PROPERTIES
    OUTPUT_NAME "pubgcheat"
    PREFIX "lib"
)

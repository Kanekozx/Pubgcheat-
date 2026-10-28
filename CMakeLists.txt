cmake_minimum_required(VERSION 3.15)
project(pubgcheat CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O2 -s -Wno-unused-function")

include(FetchContent)
FetchContent_Declare(
    xhook
    GIT_REPOSITORY https://github.com/iqiyi/xHook.git
    GIT_TAG        v1.2.0
)
FetchContent_MakeAvailable(xhook)

# --- ImGui ---
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
# Явная проверка — сразу увидишь, чего не хватает
foreach(f ${IMGUI_SOURCES})
    if(NOT EXISTS "${f}")
        message(FATAL_ERROR "imgui source missing: ${f}")
    endif()
endforeach()

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# --- Модуль ---
add_library(pubgcheat SHARED
    src/module.cpp          # ← было module.cpp
    src/sdk.cpp
    src/menu.cpp
)

target_include_directories(pubgcheat PRIVATE
    src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${xhook_SOURCE_DIR}/libxhook/jni  
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

set_target_properties(pubgcheat PROPERTIES
    OUTPUT_NAME "pubgcheat"
    PREFIX "lib"
)

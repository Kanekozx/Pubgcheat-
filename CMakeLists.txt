cmake_minimum_required(VERSION 3.15)
# ВОТ ГЛАВНОЕ ИСПРАВЛЕНИЕ: Добавлен язык C
project(pubgcheat C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O2 -s -Wno-unused-function")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -O2 -s")

# --- xhook через FetchContent ---
include(FetchContent)
FetchContent_Declare(
    xhook
    GIT_REPOSITORY https://github.com/iqiyi/xHook.git
    GIT_TAG        v1.2.0
)

FetchContent_GetProperties(xhook)
if(NOT xhook_POPULATED)
    FetchContent_Populate(xhook)
endif()

# Собираем xhook как STATIC библиотеку
add_library(xhook STATIC
    ${xhook_SOURCE_DIR}/libxhook/jni/xh_core.c
    ${xhook_SOURCE_DIR}/libxhook/jni/xh_elf.c
    ${xhook_SOURCE_DIR}/libxhook/jni/xh_jni.c
    ${xhook_SOURCE_DIR}/libxhook/jni/xh_log.c
    ${xhook_SOURCE_DIR}/libxhook/jni/xh_util.c
    ${xhook_SOURCE_DIR}/libxhook/jni/xh_version.c
    ${xhook_SOURCE_DIR}/libxhook/jni/xhook.c
)

target_include_directories(xhook PUBLIC
    ${xhook_SOURCE_DIR}/libxhook/jni
)
target_link_libraries(xhook log)

# --- ImGui ---
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/imgui")
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# --- Модуль ---
add_library(pubgcheat SHARED
    src/module.cpp
    src/sdk.cpp
    src/menu.cpp
)

target_include_directories(pubgcheat PRIVATE
    src
    ${CMAKE_CURRENT_SOURCE_DIR}
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

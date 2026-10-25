cmake_minimum_required(VERSION 3.18)
project(pubgcheat CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- xhook (замена zygisk::Api) ----
include(FetchContent)
FetchContent_Declare(xhook
    GIT_REPOSITORY https://github.com/iqiyi/xHook.git
    GIT_TAG        v1.2.2
)
FetchContent_MakeAvailable(xhook)

add_library(pubgcheat SHARED
    module.cpp
    src/sdk.cpp
    src/menu.cpp
    src/esp.cpp
)

target_include_directories(pubgcheat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

target_link_libraries(pubgcheat
    log
    android
    EGL
    GLESv3
    dl
    xhook
)

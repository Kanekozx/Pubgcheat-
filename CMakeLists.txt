cmake_minimum_required(VERSION 3.18)
project(pubg_internal CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройки для сборки Android библиотеки
add_library(main SHARED src/main.cpp)

# Ищем библиотеки Android
target_link_libraries(main
    android
    log
    GLESv3
    EGL
)

# Флаги компилятора
target_compile_options(main PRIVATE -O2 -fvisibility=hidden -Wall)

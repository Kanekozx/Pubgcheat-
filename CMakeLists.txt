cmake_minimum_required(VERSION 3.18)
project(esp_internal CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ---------- ImGui (docking) ----------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ---------- zygisk.hpp ----------
set(ZYGISK_HPP ${CMAKE_BINARY_DIR}/zygisk.hpp)
if (NOT EXISTS ${ZYGISK_HPP})
    file(DOWNLOAD
        https://raw.githubusercontent.com/topjohnwu/zygisk-module-sample/master/module/jni/zygisk.hpp
        ${ZYGISK_HPP}
        STATUS ZYGISK_STATUS
    )
    list(GET ZYGISK_STATUS 0 ZYGISK_CODE)
    if (NOT ZYGISK_CODE EQUAL 0)
        message(FATAL_ERROR "zygisk.hpp download failed: ${ZYGISK_STATUS}")
    endif()
endif()

# ---------- наш модуль ----------
add_library(esp_internal SHARED
    src/module.cpp
    src/sdk.cpp
    src/esp.cpp
    src/menu.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(esp_internal PRIVATE
    src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_BINARY_DIR}
)

target_compile_definitions(esp_internal PRIVATE
    IMGUI_IMPL_OPENGL_ES3
)

target_compile_options(esp_internal PRIVATE
    -O2 -fvisibility=hidden -fno-exceptions -fno-rtti
)

# было
target_link_libraries(esp_internal log android EGL GLESv3 dl)

# станет — добавляем xhook
add_subdirectory(xhook)  # или FetchContent_Declare
target_link_libraries(esp_internal log android EGL GLESv3 dl xhook)
target_compile_definitions(esp_internal PRIVATE PTRACE_MODE)
)

target_link_options(esp_internal PRIVATE -Wl,--no-undefined)

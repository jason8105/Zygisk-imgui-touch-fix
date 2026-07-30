cmake_minimum_required(VERSION 3.22.1)
project("zygisk")

add_library(zygisk SHARED
    zygisk.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE imgui imgui/backends)

find_library(log-lib log)
find_library(android-lib android)
find_library(glesv2-lib GLESv2)
find_library(EGL-lib EGL)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${glesv2-lib}
    ${EGL-lib}
)

cmake_minimum_required(VERSION 3.22.1)
project(zygisk_imgui)

add_library(zygisk SHARED
    zygisk.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_opengl3.cpp
)

find_library(log-lib log)
find_library(android-lib android)
find_library( EGL-lib EGL )
find_library( GLESv2-lib GLESv2 )

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${EGL-lib}
    ${GLESv2-lib}
)

target_include_directories(zygisk PRIVATE imgui)

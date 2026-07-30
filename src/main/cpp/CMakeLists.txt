cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui_touch_fix")

add_library(zygisk SHARED
    zygisk_main.cpp
    touch_hook.cpp
    imgui_impl.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_android.cpp
)

target_include_directories(zygisk PRIVATE
    .
    imgui
)

find_library(log-lib log)
find_library(android-lib android)
find_library(glesv2-lib GLESv2)
find_library( EGL-lib EGL )

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${glesv2-lib}
    ${EGL-lib}
)

target_compile_options(zygisk PRIVATE -std=c++17 -Wall -Wextra -fvisibility=hidden)

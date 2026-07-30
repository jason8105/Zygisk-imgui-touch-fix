cmake_minimum_required(VERSION 3.22.1)
project(zygisk)

add_library(zygisk SHARED
    zygisk.cpp
    input_hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_android.cpp
)

target_include_directories(zygisk PRIVATE imgui)
target_link_libraries(zygisk log android EGL GLESv3)

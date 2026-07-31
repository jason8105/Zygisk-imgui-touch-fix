cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    .
    imgui
)

add_library(
    zygisk
    SHARED
    main.cpp
    touch.cpp
    draw.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_impl_opengl3.cpp
)

target_link_libraries(
    zygisk
    android
    EGL
    GLESv3
    log
    dl
)

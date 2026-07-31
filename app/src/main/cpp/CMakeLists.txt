cmake_minimum_required(VERSION 3.22.1)
project("zygisk")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    .
    gui
    hooks
    imgui
)

add_library(zygisk SHARED
    main.cpp
    gui/menu.cpp
    hooks/hook_utils.cpp
    hooks/touch_hook.cpp
    hooks/egl_hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_impl_gles3.cpp
)

target_link_libraries(zygisk
    EGL
    GLESv3
    android
    log
    dl
)

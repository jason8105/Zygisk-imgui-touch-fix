cmake_minimum_required(VERSION 3.22.1)
project(zygisk_imgui_touch_fix)

add_library(zygisk SHARED
    zygisk.cpp
    imgui_hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
)

target_include_directories(zygisk PRIVATE
    imgui
    include
)

target_link_libraries(zygisk PRIVATE
    android
    log
    EGL
    GLESv2
)

target_compile_definitions(zygisk PRIVATE
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
)

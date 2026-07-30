cmake_minimum_required(VERSION 3.22.1)
project("zygisk_touch_fix")

add_library(zygisk_touch_fix SHARED
    native_lib.cpp
    # ImGui sources if included directly, or link prebuilt/submodule
    ../../../../../imgui/imgui.cpp
    ../../../../../imgui/imgui_demo.cpp
    ../../../../../imgui/imgui_draw.cpp
    ../../../../../imgui/imgui_tables.cpp
    ../../../../../imgui/imgui_widgets.cpp
    ../../../../../imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk_touch_fix PRIVATE
    ../../../../../imgui
    ../../../../../imgui/backends
    include
)

find_library(log-lib log)
find_library(gles-lib GLESv2)

target_link_libraries(zygisk_touch_fix
    ${log-lib}
    ${gles-lib}
    android
)

cmake_minimum_required(VERSION 3.22.1)
project("zygiskimgui")

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

target_include_directories(zygisk PRIVATE
    .
    imgui
)

find_library(log-lib log)
find_library(android-lib android)
find_library( GLESv2-lib GLESv2 )

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${GLESv2-lib}
)

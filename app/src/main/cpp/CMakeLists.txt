cmake_minimum_required(VERSION 3.22.1)
project(zygisk_imgui)

add_library(zygisk SHARED
    main.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    imgui
    zygisk
)

find_library(log-lib log)
find_library(android-lib android)
find_library(gles-lib GLESv3)
find_library(EGL-lib EGL)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${gles-lib}
    ${EGL-lib}
)

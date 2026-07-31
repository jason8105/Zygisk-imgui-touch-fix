cmake_minimum_required(VERSION 3.22.1)

project("zygisk")

add_library(
        zygisk
        SHARED
        main.cpp
        touch/touch_hook.cpp
        hooks/graphics_hook.cpp
        dobby/dobby.cpp
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_opengl3.cpp
        imgui/backends/imgui_impl_android.cpp
)

target_include_directories(
        zygisk
        PRIVATE
        .
        touch
        hooks
        dobby
        imgui
        imgui/backends
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles3-lib GLESv3)

target_link_libraries(
        zygisk
        ${log-lib}
        ${android-lib}
        ${egl-lib}
        ${gles3-lib}
)

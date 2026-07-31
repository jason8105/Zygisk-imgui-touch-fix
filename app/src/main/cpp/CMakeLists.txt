cmake_minimum_required(VERSION 3.22.1)
project("zygisk")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zygisk SHARED
    main.cpp
    hook_utils.cpp
    render_hook.cpp
    touch_hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_android.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

target_link_libraries(zygisk
    log
    android
    EGL
    GLESv2
    GLESv3
    dl
)

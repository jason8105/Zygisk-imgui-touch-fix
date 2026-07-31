cmake_minimum_required(VERSION 3.18.1)
project("zygisk_imgui_menu")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zygisk SHARED
    zygisk/zygisk_main.cpp
    touch/touch_hook.cpp
    render/render_hook.cpp
    hook/plt_hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_impl_android.cpp
    imgui/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/zygisk
    ${CMAKE_CURRENT_SOURCE_DIR}/touch
    ${CMAKE_CURRENT_SOURCE_DIR}/render
    ${CMAKE_CURRENT_SOURCE_DIR}/hook
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

target_link_libraries(zygisk
    android
    EGL
    GLESv3
    log
    dl
)

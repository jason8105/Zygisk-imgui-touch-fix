cmake_minimum_required(VERSION 3.22.1)
project("zygisk")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

add_library(zygisk SHARED
    main.cpp
    plt_hook.cpp
    touch_hook.cpp
    imgui_manager.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_android.cpp
)

target_link_libraries(zygisk
    android
    EGL
    GLESv3
    log
    dl
)

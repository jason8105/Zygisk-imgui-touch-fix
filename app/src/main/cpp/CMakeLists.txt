cmake_minimum_required(VERSION 3.18.1)
project("zygisk")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zygisk SHARED
    main.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(glesv2-lib GLESv2)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${glesv2-lib}
    dl
)

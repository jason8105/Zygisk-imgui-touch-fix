cmake_minimum_required(VERSION 3.22.1)
project("zygisk")

set(CMAKE_CXX_STANDARD 17)

add_library(zygisk SHARED
    main.cpp
    hooks/input_hook.cpp
    hooks/egl_hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/hooks
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles2-lib GLESv2)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${gles2-lib}
)

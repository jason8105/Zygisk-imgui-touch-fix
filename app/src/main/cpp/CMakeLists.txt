cmake_minimum_required(VERSION 3.18.1)
project("zygisk_imgui")

set(CMAKE_CXX_STANDARD 17)

add_library(zygisk SHARED
    main.cpp
    hooks/hook_utils.cpp
    hooks/graphics_hook.cpp
    hooks/touch_hook.cpp
    gui/menu.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_gles3.cpp
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles3-lib GLESv3)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/hooks
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${gles3-lib}
)

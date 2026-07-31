cmake_minimum_required(VERSION 3.22.1)
project("zygisk")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3 -fvisibility=hidden -fomit-frame-pointer)
add_definitions(-DIMGUI_IMPL_OPENGL_ES3)

add_library(zygisk SHARED
    main.cpp
    hook/hook_engine.cpp
    gui/menu.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/hook
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

find_library(log-lib log)
find_library(egl-lib EGL)
find_library(gles3-lib GLESv3)
find_library(android-lib android)

target_link_libraries(zygisk
    ${log-lib}
    ${egl-lib}
    ${gles3-lib}
    ${android-lib}
)

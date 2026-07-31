cmake_minimum_required(VERSION 3.18.1)
project(zygisk_imgui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../native_libs/${ANDROID_ABI}")

include_directories(
    .
    imgui
    imgui/backends
)

add_library(
    zygisk
    SHARED
    main.cpp
    hook.cpp
    touch.cpp
    menu.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

find_library(log-lib log)
find_library(egl-lib EGL)
find_library(glesv3-lib GLESv3)
find_library(android-lib android)

target_link_libraries(
    zygisk
    ${log-lib}
    ${egl-lib}
    ${glesv3-lib}
    ${android-lib}
)

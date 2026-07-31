cmake_minimum_required(VERSION 3.18.1)
project("zygisk_imgui")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main/cpp/main.cpp
    src/main/cpp/hooks.cpp
    src/main/cpp/dobby/dobby.cpp
    src/main/cpp/imgui/imgui.cpp
    src/main/cpp/imgui/imgui_draw.cpp
    src/main/cpp/imgui/imgui_widgets.cpp
    src/main/cpp/imgui/imgui_tables.cpp
    src/main/cpp/imgui/imgui_demo.cpp
    src/main/cpp/imgui/imgui_impl_opengl3.cpp
    src/main/cpp/imgui/imgui_impl_android.cpp
)

add_library(zygisk SHARED ${SOURCES})

target_include_directories(zygisk PRIVATE
    src/main/cpp
    src/main/cpp/dobby
    src/main/cpp/imgui
)

find_library(log-lib log)
find_library(android-lib android)
find_library(gles3-lib GLESv3)
find_library(egl-lib EGL)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${gles3-lib}
    ${egl-lib}
    dl
)

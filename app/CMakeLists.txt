cmake_minimum_required(VERSION 3.22.1)
project("zygisk")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/target_libs/${ANDROID_ABI}")

include_directories(
    src/main/cpp
    src/main/cpp/imgui
    src/main/cpp/imgui/backends
    src/main/cpp/dobby
)

file(GLOB IMGUI_SOURCES
    src/main/cpp/imgui/*.cpp
    src/main/cpp/imgui/backends/*.cpp
)

add_library(zygisk SHARED
    src/main/cpp/main.cpp
    src/main/cpp/hooks/hooks.cpp
    src/main/cpp/touch/touch_hook.cpp
    src/main/cpp/gui/gui.cpp
    src/main/cpp/dobby/dobby.cpp
    ${IMGUI_SOURCES}
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles3-lib GLESv3)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${gles3-lib}
)

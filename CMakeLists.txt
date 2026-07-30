cmake_minimum_required(VERSION 3.22.1)
project(zygisk_imgui_touch_fix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zygisk SHARED
    src/main/cpp/main.cpp
)

find_library(log-lib log)
find_library(android-lib android)
find_library(EGL-lib EGL)
find_library(GLESv2-lib GLESv2)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${EGL-lib}
    ${GLESv2-lib}
)

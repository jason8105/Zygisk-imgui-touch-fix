cmake_minimum_required(VERSION 3.22.1)
project(zygisk_imgui_touch_fix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zygisk SHARED
    zygisk_module.cpp
    imgui_integration.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
)

cmake_minimum_required(VERSION 3.22.1)
project(zygisk_imgui)

add_library(zygisk SHARED
    zygisk_main.cpp
    imgui_hook.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
)

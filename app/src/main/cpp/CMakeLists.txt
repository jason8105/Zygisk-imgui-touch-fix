cmake_minimum_required(VERSION 3.22.1)
project(zygisk_module CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zygisk SHARED
    main.cpp
    imgui/imgui.cpp
)

target_include_directories(zygisk PRIVATE
    .
    imgui
)

target_link_libraries(zygisk
    android
    log
)

cmake_minimum_required(VERSION 3.22.1)
project(zygisk_mod CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(zygisk SHARED
    main.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
)

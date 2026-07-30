cmake_minimum_required(VERSION 3.22.1)
project(zyCheats CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Zygisk headers & Dobby if available or standard paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Find required libraries
find_library(log-lib log)
find_library(android-lib android)

# ImGui sources (assuming standard dear imgui files are present or compiled here)
file(GLOB IMGUI_SOURCES
    imgui/*.cpp
)

add_library(zyCheats SHARED
    main.cpp
    ${IMGUI_SOURCES}
)

target_link_libraries(zyCheats
    ${log-lib}
    ${android-lib}
)

cmake_minimum_required(VERSION 3.22.1)
project(zygisk_imgui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Include Zygisk headers (assumed or stubbed in repo or included via path)
# If zygisk.hpp is in the same directory, it will be found.

add_library(zygisk_imgui SHARED
    zygisk_module.cpp
    universal_touch.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(zygisk_imgui
    ${log-lib}
    ${android-lib}
)

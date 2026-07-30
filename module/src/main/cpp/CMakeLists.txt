cmake_minimum_required(VERSION 3.22.1)
project(zygisk_touch_fix LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add Dobby if available or mock/include headers if bundled
# Assuming Dobby headers and static/shared libs are placed under third_party/dobby or similar,
# or we define standard include paths. Let's make sure dobby.h is accessible.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(zygisk_touch_fix SHARED
    main.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(zygisk_touch_fix
    ${log-lib}
    ${android-lib}
)

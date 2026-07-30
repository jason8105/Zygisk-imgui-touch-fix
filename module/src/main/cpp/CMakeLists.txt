cmake_minimum_required(VERSION 3.22.1)
project(zygisk_touch_fix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Dobby and ImGui headers/sources if available, or build inline
add_library(zygisk_touch_fix SHARED
    main.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(zygisk_touch_fix
    ${log-lib}
    ${android-lib}
)

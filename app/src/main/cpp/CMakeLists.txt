cmake_minimum_required(VERSION 3.22.1)
project("zygisk")

add_library(zygisk SHARED
    zygisk.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
)
=== END FILE: app/src/main/cpp/CMakeLists.txt ===

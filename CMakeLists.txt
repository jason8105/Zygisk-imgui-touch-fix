cmake_minimum_required(VERSION 3.10.2)
project(zygisk_mod)

add_library(zygisk SHARED 
    src/main/cpp/main.cpp 
    src/main/cpp/hook.cpp
)

target_link_libraries(zygisk android log)
set_target_properties(zygisk PROPERTIES LINK_FLAGS "-Wl,--exclude-libs,ALL")

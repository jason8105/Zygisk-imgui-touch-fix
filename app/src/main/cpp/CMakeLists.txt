cmake_minimum_required(VERSION 3.10.2)
project(zygisk_mod)

add_library(zygisk SHARED 
    main.cpp 
    input_hook.cpp
)

target_link_libraries(zygisk android log)
# Ensure symbols are exported for Zygisk
set_target_properties(zygisk PROPERTIES LINK_FLAGS "-Wl,--build-id -Wl,--no-undefined")

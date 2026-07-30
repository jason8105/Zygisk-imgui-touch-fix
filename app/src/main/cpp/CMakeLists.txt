cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui")

add_library(zygisk SHARED
    zygisk_module.cpp
    imgui_impl.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
)

target_compile_definitions(zygisk PRIVATE ZYGISK_SDK=1)

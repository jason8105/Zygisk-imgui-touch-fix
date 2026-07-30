cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui_touch_fix")

add_library(zygisk_imgui_touch_fix SHARED
    main.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(zygisk_imgui_touch_fix
    ${log-lib}
    ${android-lib}
)

target_compile_features(zygisk_imgui_touch_fix PRIVATE cxx_std_17)
target_compile_definitions(zygisk_imgui_touch_fix PRIVATE
    __ANDROID_API__=26
)

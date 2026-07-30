cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui_touch_fix")

add_library(zygisk SHARED
    zygisk_entry.cpp
    imgui_hook.cpp
)

find_library(log-lib log)
find_library(android-lib android)
find_library(gles-lib GLESv2)
find_library(EGL-lib EGL)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${gles-lib}
    ${EGL-lib}
)

target_compile_definitions(zygisk PRIVATE
    IMGUI_IMPL_OPENGL_ES3
)

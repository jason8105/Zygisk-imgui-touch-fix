cmake_minimum_required(VERSION 3.18.1)
project("zygisk_imgui")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../build/zygisk_libs/${ANDROID_ABI}")
file(MAKE_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

add_library(zygisk SHARED
    main.cpp
    hooks/touch_hook.cpp
    gui/gui.cpp
    dobby/dobby.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles3-lib GLESv3)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${gles3-lib}
    dl
)

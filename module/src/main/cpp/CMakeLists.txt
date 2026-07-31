cmake_minimum_required(VERSION 3.22.1)
project(zygisk CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../zygisk_libs/${ANDROID_ABI})

add_library(zygisk SHARED
    main.cpp
    touch_hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    .
    imgui
    imgui/backends
)

target_link_libraries(zygisk
    android
    log
    EGL
    GLESv3
    dl
)

add_custom_command(TARGET zygisk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/../module/zygisk/${ANDROID_ABI}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:zygisk>" "${CMAKE_CURRENT_SOURCE_DIR}/../module/zygisk/${ANDROID_ABI}/libzygisk.so"
    COMMENT "Copying libzygisk.so to target module directory for ${ANDROID_ABI}"
)

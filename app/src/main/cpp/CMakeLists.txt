cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    hook.cpp
    touch.cpp
    gui.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_android.cpp
)

add_library(zygisk SHARED ${SOURCES})

target_include_directories(zygisk PRIVATE
    .
    imgui
    imgui/backends
)

target_link_libraries(zygisk
    android
    EGL
    GLESv3
    log
)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET zygisk POST_BUILD
        COMMAND ${CMAKE_STRIP} --strip-unneeded $<TARGET_FILE:zygisk>
    )
endif()

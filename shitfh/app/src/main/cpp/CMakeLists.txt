cmake_minimum_required(VERSION 3.22.1)
project(zygisk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_android.cpp
)

set(SOURCES
    main.cpp
    hooks/plt_hook.cpp
    hooks/input_hook.cpp
    hooks/egl_hook.cpp
    ${IMGUI_SOURCES}
)

add_library(zygisk SHARED ${SOURCES})

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/zygisk
    ${CMAKE_CURRENT_SOURCE_DIR}/hooks
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(zygisk
    log
    android
    EGL
    GLESv3
    dl
)

set_target_properties(zygisk PROPERTIES
    LINK_FLAGS "-Wl,--gc-sections -Wl,--exclude-libs,ALL"
)

cmake_minimum_required(VERSION 3.18.1)
project("zygisk")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

add_library(zygisk SHARED
    main.cpp
    dobby/dobby.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dobby
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
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
)

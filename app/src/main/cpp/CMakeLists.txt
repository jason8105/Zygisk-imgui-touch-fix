cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui" C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

add_library(zygisk SHARED
    main.cpp
    touch.cpp
    hook_utils.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_android.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

find_library(log-lib log)
find_library(egl-lib EGL)
find_library(glesv3-lib GLESv3)
find_library(android-lib android)

target_link_libraries(zygisk
    ${log-lib}
    ${egl-lib}
    ${glesv3-lib}
    ${android-lib}
    dl
)

cmake_minimum_required(VERSION 3.18.1)
project(zygisk_touch_fix C CXX)

# Fix: Corrected pathing to point to project root/submodules correctly
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

add_library(zygisk_touch_fix SHARED
    src/main.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
)

target_include_directories(zygisk_touch_fix PRIVATE ${IMGUI_DIR})
target_link_libraries(zygisk_touch_fix android log)

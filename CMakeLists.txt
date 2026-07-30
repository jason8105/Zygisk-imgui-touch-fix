cmake_minimum_required(VERSION 3.18.1)
project(zygisk_imgui_touch_fix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for ImGui and Dobby if needed
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/imgui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/dobby/include)

add_library(zygisk_imgui SHARED
    src/main/cpp/main.cpp
    src/main/cpp/imgui/imgui.cpp
    src/main/cpp/imgui/imgui_draw.cpp
    src/main/cpp/imgui/imgui_tables.cpp
    src/main/cpp/imgui/imgui_widgets.cpp
    src/main/cpp/imgui/imgui_impl_opengl3.cpp
    src/main/cpp/imgui/imgui_impl_android.cpp
)

find_library(log-lib log)
find_library(android-lib android)
find_library(glesv2-lib GLESv2)
find_library(EGL-lib EGL)

target_link_libraries(zygisk_imgui
    ${log-lib}
    ${android-lib}
    ${glesv2-lib}
    ${EGL-lib}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/dobby/lib/arm64-v8a/libdobby.a
)

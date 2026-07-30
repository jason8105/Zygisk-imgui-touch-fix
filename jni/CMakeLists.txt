cmake_minimum_required(VERSION 3.18.1)

project("zygisk_imgui")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    include
    imgui
    external/dobby/include
)

# Source files
file(GLOB IMGUI_SRC "imgui/*.cpp" "imgui/backends/imgui_impl_android.cpp" "imgui/backends/imgui_impl_opengl3.cpp")

add_library(imgui_menu SHARED
    main.cpp
    ${IMGUI_SRC}
)

# Link libraries
target_link_libraries(imgui_menu
    android
    log
    EGL
    GLESv3
    ${CMAKE_SOURCE_DIR}/external/dobby/lib/$(ANDROID_ABI)/libdobby.a
)

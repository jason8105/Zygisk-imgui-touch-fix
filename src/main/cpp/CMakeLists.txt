cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui")

# Set output directory to a predictable location for Gradle
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/outputs)

add_library(zygisk SHARED
        main.cpp
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_demo.cpp
        imgui/backends/imgui_impl_android.cpp
        imgui/backends/imgui_impl_opengl3.cpp
        # Add your backend files here
)

target_link_libraries(zygisk
        android
        log
        EGL
        GLESv3
)

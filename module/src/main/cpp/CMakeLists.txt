cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui")

# Add your ImGui sources and Dobby (for hooking)
# Assuming dobby and imgui are in the same directory or submodules
add_library(zygisk SHARED
    main.cpp
    input_hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_android.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# Use find_library for NDK logs and android libs
find_library(log-lib log)
find_library(android-lib android)
find_library(gles3-lib GLESv3)

target_include_directories(zygisk PRIVATE . imgui imgui/backends dobby/include)
target_link_libraries(zygisk ${log-lib} ${android-lib} ${gles3-lib} dobby)

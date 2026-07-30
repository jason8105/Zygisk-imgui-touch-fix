cmake_minimum_required(VERSION 3.22.1)
project("zygiskimgui")

# Note: In a real environment, you'd add ImGui and Dobby source files here.
# This setup assumes standard directory structure.

add_library(zygiskimgui SHARED
    main.cpp
    # Add your ImGui sources here:
    # imgui/imgui.cpp
    # imgui/imgui_draw.cpp
    # ...
)

target_link_libraries(zygiskimgui
    android
    log
    EGL
    GLESv3
)

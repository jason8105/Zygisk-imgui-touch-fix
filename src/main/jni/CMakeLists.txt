cmake_minimum_required(VERSION 3.18.1)
project("zygiskimgui")

# Include ImGui and Dobby (ensure these paths match your repo structure)
# For this fix, we assume standard ImGui and Dobby setup
add_library(zygisk SHARED
        main.cpp
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/backends/imgui_impl_android.cpp
        imgui/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(zygisk
        android
        log
        EGL
        GLESv3
        Dobby # Required for Universal Touch Hooking
)

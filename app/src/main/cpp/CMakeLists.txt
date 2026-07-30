cmake_minimum_required(VERSION 3.22.1)

project("zygisk_imgui")

# Note: You should have ImGui sources and Dobby in your source tree or as submodules
# For this implementation, we assume they are in the 'vendor' directory
set(IMGUI_DIR vendor/imgui)
set(DOBBY_DIR vendor/Dobby)

add_library(zygisk SHARED
        main.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_android.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
        .
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${DOBBY_DIR}/include
)

target_link_libraries(zygisk
        android
        log
        EGL
        GLESv3
        ${DOBBY_DIR}/build/libdobby.a # Ensure Dobby is precompiled or added as a subproject
)

set_target_properties(zygisk PROPERTIES OUTPUT_NAME "zygisk")

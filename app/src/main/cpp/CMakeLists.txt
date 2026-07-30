cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui")

# Add your ImGui and Dobby paths here
# Assuming imgui is in a subdirectory 'imgui' and dobby in 'dobby'
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(DOBBY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dobby)

add_library(zygisk SHARED
        main.cpp
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
        ${DOBBY_DIR}/include
)

target_link_libraries(zygisk
        android
        log
        GLESv3
        EGL
        ${DOBBY_DIR}/lib/libdobby.a
)

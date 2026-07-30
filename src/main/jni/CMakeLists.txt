cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui")

# Add ImGui files (assumed to be in imgui/ folder)
set(IMGUI_DIR imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_android.h
    ${IMGUI_DIR}/backends/imgui_impl_android.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(zygisk SHARED
    main.cpp
    hooks.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(zygisk PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(zygisk
    android
    log
    EGL
    GLESv3
)

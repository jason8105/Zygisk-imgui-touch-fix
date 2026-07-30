cmake_minimum_required(VERSION 3.18.1)

project("zygisk_imgui")

# Add ImGui sources (assuming they are in jni/imgui)
set(IMGUI_DIR imgui)
file(GLOB IMGUI_SOURCES 
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_android.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(zygisk_imgui SHARED
    main.cpp
    hooks.cpp
    ${IMGUI_SOURCES}
)

# Include Dobby for hooking (Assuming Dobby is in jni/Dobby)
include_directories(include Dobby/include ${IMGUI_DIR} ${IMGUI_DIR}/backends)

target_link_libraries(zygisk_imgui
    android
    log
    EGL
    GLESv3
    ${CMAKE_CURRENT_SOURCE_DIR}/Dobby/build/libdobby.a
)

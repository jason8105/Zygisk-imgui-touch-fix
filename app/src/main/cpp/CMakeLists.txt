cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui")

# Dobby for hooking
add_subdirectory(dobby)

# ImGui files (assumed to be in imgui/ folder)
set(IMGUI_DIR imgui)
file(GLOB IMGUI_SOURCES 
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_android.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_library(zygisk SHARED 
    main.cpp
    hooks.cpp
    renderer.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(zygisk PRIVATE 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    dobby/include
)

target_link_libraries(zygisk 
    dobby 
    GLESv3 
    EGL 
    android 
    log
)

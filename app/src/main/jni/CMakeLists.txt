cmake_minimum_required(VERSION 3.22.1)
project("zygisk_imgui")

# Add your ImGui sources here
set(IMGUI_DIR imgui)
file(GLOB IMGUI_SOURCES 
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_android.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(zygisk SHARED
    main.cpp
    ${IMGUI_SOURCES}
)

# Link Dobby or a similar hooking library for the touch fix
# You can use FetchContent or a local path. For brevity, we assume standard linking.
target_link_libraries(zygisk
    android
    log
    EGL
    GLESv3
)

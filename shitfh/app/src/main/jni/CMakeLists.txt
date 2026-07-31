cmake_minimum_required(VERSION 3.18.1)

project(zygisk_imgui CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch Dear ImGui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.89.9
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
endif()

# Define the source files for our Zygisk module
set(MODULE_SOURCES
    main.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(zygisk SHARED ${MODULE_SOURCES})

# Include directories
target_include_directories(zygisk PRIVATE
    .
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# OpenGLES 3 Definition for ImGui
target_compile_definitions(zygisk PRIVATE IMGUI_IMPL_OPENGL_ES3)

# Link necessary libraries
target_link_libraries(zygisk PRIVATE
    log
    android
    EGL
    GLES3
)

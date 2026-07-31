cmake_minimum_required(VERSION 3.18.1)
project("zygisk_imgui")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
)

set(DOBBY_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_GetProperties(dobby)
if(NOT dobby_POPULATED)
    FetchContent_Populate(dobby)
    add_subdirectory(${dobby_SOURCE_DIR} ${dobby_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

include_directories(
    src/main/cpp
    src/main/cpp/imgui
    ${dobby_SOURCE_DIR}/include
)

add_library(zygisk SHARED
    main.cpp
    hook_engine.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_impl_opengl3.cpp
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles-lib GLESv3)

target_link_libraries(zygisk
    dobby
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${gles-lib}
)

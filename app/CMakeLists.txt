cmake_minimum_required(VERSION 3.18.1)
project(zygisk_imgui C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/imgui_impl_android.cpp
)

add_library(zygisk SHARED
    src/main/cpp/main.cpp
    src/main/cpp/touch_hook.cpp
    src/main/cpp/imgui_manager.cpp
    src/main/cpp/dobby_stub.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(zygisk PRIVATE
    src/main/cpp
    ${IMGUI_DIR}
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles3-lib GLESv3)

target_link_libraries(zygisk
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${gles3-lib}
    dl
)

set(MODULE_ZYGISK_DIR "${CMAKE_SOURCE_DIR}/../module/zygisk/${ANDROID_ABI}")

add_custom_command(TARGET zygisk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${MODULE_ZYGISK_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:zygisk> "${MODULE_ZYGISK_DIR}/libzygisk.so"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:zygisk> "${MODULE_ZYGISK_DIR}/../${ANDROID_ABI}.so"
    COMMENT "Copying compiled libzygisk.so to module directory: ${MODULE_ZYGISK_DIR}"
)

cmake_minimum_required(VERSION 3.22.1)
project(zygisk_imgui CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zygisk SHARED
    main.cpp
    hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
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
)

# Fix module output structure: Copy libzygisk.so to src/main/module_out/zygisk/${ANDROID_ABI}/libzygisk.so
add_custom_command(TARGET zygisk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/../module_out/zygisk/${ANDROID_ABI}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:zygisk>" "${CMAKE_CURRENT_SOURCE_DIR}/../module_out/zygisk/${ANDROID_ABI}/libzygisk.so"
    COMMENT "Copied libzygisk.so for ABI ${ANDROID_ABI} into module output directory"
)

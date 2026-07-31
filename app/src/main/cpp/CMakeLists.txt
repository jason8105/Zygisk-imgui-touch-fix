cmake_minimum_required(VERSION 3.18.1)
project("zygisk_imgui" CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../out/zygisk/${ANDROID_ABI}")
set(TEMPLATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../template/zygisk/${ANDROID_ABI}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

add_library(zygisk SHARED
    main.cpp
    hook.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_android.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(zygisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

target_link_libraries(zygisk
    android
    EGL
    GLESv3
    log
)

add_custom_command(TARGET zygisk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${TEMPLATE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:zygisk>" "${TEMPLATE_DIR}/libzygisk.so"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:zygisk>" "${OUTPUT_DIR}/libzygisk.so"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:zygisk>" "${OUTPUT_DIR}/../${ANDROID_ABI}.so"
)

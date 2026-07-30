cmake_minimum_required(VERSION 3.22.1)
project("zygiskimgui")

# Include Dobby and ImGui paths here
# Assuming ImGui and Dobby are in subdirectories or linked via local includes
add_library(zygisk SHARED main.cpp)

target_link_libraries(zygisk log android)

# Ensure the library is named libzygisk.so as required by Magisk
set_target_properties(zygisk PROPERTIES OUTPUT_NAME "zygisk")

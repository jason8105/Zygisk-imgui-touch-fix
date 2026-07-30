```cmake
cmake_minimum_required(VERSION 3.18.1)
project(zygisk_menu)

add_library(zygisk SHARED main.cpp)
target_link_libraries(zygisk android log)

set_target_properties(zygisk PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/obj/${ANDROID_ABI}"
)
```

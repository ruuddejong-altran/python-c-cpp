project("Python-C-C++ Example 1")

cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(spamlib SHARED spamlib.c)

add_executable(demo main.c)
target_link_libraries(demo spamlib)

add_custom_target(copy_demo ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/demo.py ${CMAKE_CURRENT_BINARY_DIR})

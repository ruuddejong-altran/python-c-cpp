project("Python-C-C++ Example 1")

cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(spamlib SHARED spamlib.c)

add_executable(demo main.c)
target_link_libraries(demo spamlib)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/demo.py ${CMAKE_CURRENT_BINARY_DIR}/spam.py
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/demo.py ${CMAKE_CURRENT_SOURCE_DIR}/spam.py ${CMAKE_CURRENT_BINARY_DIR}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/demo.py)

add_custom_target(copy_demo DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/demo.py ${CMAKE_CURRENT_BINARY_DIR}/spam.py)

add_dependencies(demo copy_demo)
add_dependencies(spamlib copy_demo)

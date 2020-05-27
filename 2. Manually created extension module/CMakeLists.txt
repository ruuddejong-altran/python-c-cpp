project("Python-C-C++ Example 2")

cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Python3 COMPONENTS Development)

if (Python3_Development_FOUND)
    message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
    message(STATUS "Python3_LIBRARIES: ${Python3_LIBRARIES}")
else (Python3_Development_FOUND)
    message(ERROR "Python3 development files not found")
endif (Python3_Development_FOUND)

add_library(spamlib SHARED spamlib.c)

Python3_add_library(spam MODULE spammodule.c)
target_link_libraries(spam PRIVATE spamlib)

add_executable(demo main.c)
target_link_libraries(demo spamlib)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/demo.py
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/demo.py ${CMAKE_CURRENT_BINARY_DIR}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/demo.py)

add_custom_target(copy_demo DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/demo.py)

add_dependencies(demo copy_demo)
add_dependencies(spamlib copy_demo)

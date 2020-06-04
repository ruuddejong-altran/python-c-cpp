project("Python-C-C++ Example 2")

cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Python3 COMPONENTS Development)

if (Python3_Development_FOUND)
    message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
    message(STATUS "Python3_LIBRARIES: ${Python3_LIBRARIES}")
else (Python3_Development_FOUND)
    message(ERROR "Python3 development files not found")
endif (Python3_Development_FOUND)

find_package(SWIG 4.0)
if (SWIG_FOUND)
    include(${SWIG_USE_FILE})
    message(STATUS "Found SWIG version ${SWIG_VERSION}")
else (SWIG_FOUND)
    message(ERROR "SWIG not found")
endif (SWIG_FOUND)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Python3_INCLUDE_DIRS})

add_library(spamlib SHARED spamlib.cpp)

add_executable(demo main.cpp)
target_link_libraries(demo spamlib)

set_property(SOURCE spam.i PROPERTY
        CPLUSPLUS ON
        DEPENDS spamlib)

swig_add_library(spam TYPE SHARED LANGUAGE python
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
        SOURCES spam.i)
swig_link_libraries(spam spamlib ${Python3_LIBRARIES})

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/demo.py
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/demo.py ${CMAKE_CURRENT_BINARY_DIR}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/demo.py)

add_custom_target(copy_demo DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/demo.py)

add_dependencies(demo copy_demo)
add_dependencies(spamlib copy_demo)
add_dependencies(spam copy_demo)

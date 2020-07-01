project("Python-C-C++ Playground")

cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Python3 COMPONENTS Development)

if (Python3_Development_FOUND)
    message(STATUS "Python executable: ${Python3_EXECUTABLE}")
    message(STATUS "Python version: ${Python3_VERSION}")
    message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
    message(STATUS "Python3_LIBRARIES: ${Python3_LIBRARIES}")
else (Python3_Development_FOUND)
    message(ERROR "Python3 development files not found")
endif (Python3_Development_FOUND)

#find_package(Threads REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Python3_INCLUDE_DIRS})

add_subdirectory(../pybind11 ${CMAKE_CURRENT_BINARY_DIR}/pybind11)

add_library(playgroundlib SHARED example.cpp)
#target_link_libraries(playgroundlib Threads::Threads)

pybind11_add_module(playground MODULE playground.cpp)
target_link_libraries(playground PRIVATE playgroundlib)

pybind11_add_module(conversion MODULE playground.cpp)
target_link_libraries(conversion PRIVATE playgroundlib)

add_executable(demo main.cpp)
target_link_libraries(demo playgroundlib)

add_custom_target(copy_demo ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/demo.py ${CMAKE_CURRENT_BINARY_DIR})

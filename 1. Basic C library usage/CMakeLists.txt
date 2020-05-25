cmake_minimum_required(VERSION 3.15)

project(greeting)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(greeting SHARED greeting.c)

add_executable(main main.c)
target_link_libraries(main greeting)


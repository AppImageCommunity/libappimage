cmake_minimum_required(VERSION 3.2)

project(libappimage)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

include(cmake/tools.cmake)
include(cmake/dependencies.cmake)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tests)

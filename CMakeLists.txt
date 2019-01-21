cmake_minimum_required(VERSION 3.2)

project(libappimage)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# versioning
set(V_MAJOR 0)
set(V_MINOR 1)
set(V_PATCH 8)

include(cmake/tools.cmake)
include(cmake/dependencies.cmake)

# used by e.g., Debian packaging infrastructure
include(GNUInstallDirs)

add_subdirectory(lib)
add_subdirectory(src)

include(CTest)
add_subdirectory(tests)

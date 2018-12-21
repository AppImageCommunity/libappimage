cmake_minimum_required(VERSION 3.2)

project(libappimage)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

include(cmake/tools.cmake)
include(cmake/dependencies.cmake)

if(MEASURE_CODE_COVERAGE)
    message("Configuring project for code coverage mesurement")
    include(CodeCoverage)
    append_coverage_compiler_flags()
endif()

add_subdirectory(lib)
add_subdirectory(src)

include(CTest)
add_subdirectory(tests)

cmake_minimum_required(VERSION 3.2)

project(libappimage)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimize for size in release builds
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG -Wl,--gc-sections")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

include(cmake/tools.cmake)
include(cmake/dependencies.cmake)

option(LIBAPPIMAGE_DESKTOP_INTEGRATION "Enable desktop integration functions" On)
option(LIBAPPIMAGE_THUMBNAILER "Enable thumbnailer functions" On)
option(LIBAPPIMAGE_STANDALONE "Statically embbed dependencies" Off)
option(MEASURE_CODE_COVERAGE "Enable tests code coverate target" Off)


if(MEASURE_CODE_COVERAGE)
    message("Configuring project for code coverage mesurement")
    include(CodeCoverage)
    append_coverage_compiler_flags()
endif()

add_subdirectory(lib)
add_subdirectory(src)

include(CTest)
add_subdirectory(tests)

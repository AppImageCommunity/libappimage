cmake_minimum_required(VERSION 3.4)

project(libappimage)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# help produce reproducible builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
    message(STATUS "Release build detected, enabling reproducible builds mode")
    get_filename_component(abs_source_path ${PROJECT_SOURCE_DIR} ABSOLUTE)
    file(RELATIVE_PATH rel_source_path ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

    set(map_fix ${abs_source_path}=${rel_source_path})

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdebug-prefix-map=${map_fix} -fmacro-prefix-map=${map_fix}")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fdebug-prefix-map=${map_fix} -fmacro-prefix-map=${map_fix}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_C_FLAGS} -fdebug-prefix-map=${map_fix} -fmacro-prefix-map=${map_fix}")
endif()

# versioning
set(V_MAJOR 0)
set(V_MINOR 1)
set(V_PATCH 7)

include(cmake/tools.cmake)
include(cmake/dependencies.cmake)

# used by e.g., Debian packaging infrastructure
include(GNUInstallDirs)

add_subdirectory(lib)
add_subdirectory(src)

include(CTest)
add_subdirectory(tests)

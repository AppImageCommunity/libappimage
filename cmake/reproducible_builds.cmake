# this little snippet makes sure that no absolute paths end up in the binaries built by CMake
# it will replace such paths with relative ones
# see https://reproducible-builds.org/docs/build-path/ for more information

cmake_minimum_required(VERSION 3.4)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    message(STATUS "Release build detected, enabling reproducible builds mode")
    get_filename_component(abs_source_path ${PROJECT_SOURCE_DIR} ABSOLUTE)
    file(RELATIVE_PATH rel_source_path ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

    set(map_fix ${abs_source_path}=${rel_source_path})
    set(extra_flags "-fdebug-prefix-map=${map_fix} -fmacro-prefix-map=${map_fix}")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${extra_flags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${extra_flags}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${extra_flags}")
endif()

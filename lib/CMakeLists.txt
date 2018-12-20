cmake_minimum_required(VERSION 3.0)

include(CTest)

if(BUILD_TESTING)
    if(NOT TARGET gtest)
        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gtest)
            add_subdirectory(gtest EXCLUDE_FROM_ALL)
        else()
            message(FATAL_ERROR "gtest submodule not found; please call git submodule update --init or disable the unit tests using -DBUILD_TESTING=OFF")
        endif()
    endif()
endif()


if(NOT TARGET linuxdeploy_desktopfile)
    add_subdirectory(linuxdeploy-desktopfile)
endif()

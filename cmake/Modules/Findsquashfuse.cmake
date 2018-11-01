cmake_minimum_required(VERSION 3.0)

find_path(squashfuse_H_DIR
    NAMES squashfuse.h
    HINTS ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES include include/linux
)

if(NOT squashfuse_H_DIR)
    message(FATAL_ERROR "squashfuse.h not found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(squashfuse DEFAULT_MSG SQUASHFUSE_INCLUDE_DIR SQUASHFUSE_LIBRARY_DIR)

add_library(squashfuse IMPORTED SHARED)
set_property(TARGET squashfuse PROPERTY IMPORTED_LOCATION ${squashfuse_LIBRARY})
set_property(TARGET squashfuse PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${squashfuse_H_DIR}")

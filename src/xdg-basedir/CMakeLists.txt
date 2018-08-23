cmake_minimum_required(VERSION 3.5)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# force static linking
add_library(xdg-basedir STATIC xdg-basedir.h xdg-basedir.c)
target_include_directories(xdg-basedir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

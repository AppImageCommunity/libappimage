cmake_minimum_required(VERSION 3.5)

add_library(temporarydirectory TemporaryDirectory.cpp)
target_include_directories(temporarydirectory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

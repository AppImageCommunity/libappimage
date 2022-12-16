cmake_minimum_required(VERSION 3.4)

add_library(temporarydirectory TemporaryDirectory.cpp)
target_include_directories(temporarydirectory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

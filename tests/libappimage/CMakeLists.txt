add_executable(test_libappimage_0.2 test_libappimage_0.2.cpp test_utile_magic_bytes_checker.cpp TraversalFallbackImplTests.cpp TraversalType1ImplTests.cpp)

target_include_directories(test_libappimage_0.2 PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage")
target_link_libraries(test_libappimage_0.2 libappimage_0.2 Boost libsquashfuse gtest gtest_main)

add_test(test_libappimage_0.2 test_libappimage_0.2)

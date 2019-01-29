if(LIBAPPIMAGE_DESKTOP_INTEGRATION)
    add_subdirectory(desktop_integration)
endif()

add_subdirectory(legacy)

add_executable(
    test_libappimage++

    TestLibappimage++.cpp

    core/impl/TestTraversalType1.cpp
    core/impl/TestTraversalType2.cpp

    utils/TestMagicBytesChecker.cpp
    utils/TestUtilsElf.cpp
    utils/TestIconHandle.cpp
)

target_include_directories(test_libappimage++ PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage")
target_link_libraries(test_libappimage++ libappimage Boost libsquashfuse gtest gtest_main)

add_test(test_libappimage++ test_libappimage++)

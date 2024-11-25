add_subdirectory(legacy)

if (NOT LIBAPPIMAGE_SHARED_ONLY)
    add_subdirectory(temporarydirectory)

    if(LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED)
        add_subdirectory(desktop_integration)
    endif()

    add_executable(
        test_libappimage++

        TestLibappimage++.cpp

        core/impl/TestTraversalType1.cpp
        core/impl/TestTraversalType2.cpp

        utils/TestMagicBytesChecker.cpp
        utils/TestUtilsElf.cpp
        utils/TestIconHandle.cpp
        utils/TestLogger.cpp
        utils/TestPayloadEntriesCache.cpp
        utils/TestResourcesExtractor.cpp
        utils/StringSanitizerTest.cpp
    )

    target_include_directories(test_libappimage++ PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage")
    target_link_libraries(test_libappimage++ temporarydirectory libappimage libarchive libsquashfuse XdgUtils::DesktopEntry XdgUtils::BaseDir GTest::gtest GTest::gtest_main)

    add_test(test_libappimage++ test_libappimage++)
endif()

add_executable(
    test_libappimage_desktop_integration_0.2
    test_desktop_integration_0.2.cpp
    DesktopExecEntryTokenizerTests.cpp
)

target_include_directories(
    test_libappimage_desktop_integration_0.2
    PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage"
    PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage_desktop_integration"
)

target_link_libraries(
    test_libappimage_desktop_integration_0.2
    PRIVATE Boost
    PRIVATE linuxdeploy_desktopfile
    PRIVATE appimage_desktop_integration
    PRIVATE libsquashfuse
    PRIVATE gtest
    PRIVATE gtest_main
)

add_test(test_libappimage_desktop_integration_0.2 test_libappimage_desktop_integration_0.2)

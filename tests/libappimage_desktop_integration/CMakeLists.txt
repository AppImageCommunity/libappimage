add_executable(
    TestDesktopIntegration

    TestIntegrationManager.cpp

    integrator/TestDesktopIntegration.cpp
    integrator/TestDesktopEntryEditor.cpp
    integrator/TestResourcesExtractor.cpp
)

target_include_directories(
    TestDesktopIntegration
    PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage"
    PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage_desktop_integration"
)

target_link_libraries(
    TestDesktopIntegration
    PRIVATE Boost
    PRIVATE appimage_desktop_integration
    PRIVATE libsquashfuse
    PRIVATE gtest
    PRIVATE gtest_main
    PRIVATE XdgUtils
)

add_test(TestDesktopIntegration TestDesktopIntegration)

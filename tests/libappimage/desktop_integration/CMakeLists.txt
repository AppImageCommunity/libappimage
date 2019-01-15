set(
    TestDesktopIntegrationSources

    TestIntegrationManager.cpp

    integrator/TestDesktopIntegration.cpp
    integrator/TestDesktopEntryEditor.cpp
    integrator/TestResourcesExtractor.cpp

    $<TARGET_OBJECTS:appimage_desktop_integration>
)

if(LIBAPPIMAGE_THUMBNAILER)
    set(TestDesktopIntegrationSources ${TestDesktopIntegrationSources} thumbnailer/TestThumbnailer.cpp)
endif()

add_executable(TestDesktopIntegration ${TestDesktopIntegrationSources})

target_include_directories(
    TestDesktopIntegration
    PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage"
    PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage/desktop_integration"
)

target_link_libraries(
    TestDesktopIntegration
    PRIVATE Boost
    PRIVATE libappimage
    PRIVATE libsquashfuse
    PRIVATE gtest
    PRIVATE gtest_main
    PRIVATE XdgUtils
    PRIVATE libglib
)

add_test(TestDesktopIntegration TestDesktopIntegration)

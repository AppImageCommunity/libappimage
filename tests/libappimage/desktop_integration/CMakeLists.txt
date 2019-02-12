set(
    TestDesktopIntegrationSources

    TestIntegrationManager.cpp

    integrator/TestDesktopIntegration.cpp
    integrator/TestDesktopEntryEditor.cpp
    integrator/TestResourcesExtractor.cpp

    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:appimage_utils>
    $<TARGET_OBJECTS:appimage_desktop_integration>
)

if(LIBAPPIMAGE_THUMBNAILER_ENABLED)
    set(TestDesktopIntegrationSources ${TestDesktopIntegrationSources} TestThumbnailer.cpp)
endif()

add_executable(TestDesktopIntegration ${TestDesktopIntegrationSources})

target_include_directories(
    TestDesktopIntegration
    PRIVATE "${PROJECT_SOURCE_DIR}/include"
    PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage"
    PRIVATE "${PROJECT_SOURCE_DIR}/src/libappimage/desktop_integration"
)

target_link_libraries(
    TestDesktopIntegration
    PRIVATE libappimage_shared
    PRIVATE libsquashfuse
    PRIVATE libarchive
    PRIVATE xz
    PRIVATE libzlib
    PRIVATE XdgUtils
    PRIVATE Boost::filesystem
    PUBLIC dl
    PRIVATE gtest
    PRIVATE gtest_main
)

add_test(TestDesktopIntegration TestDesktopIntegration)

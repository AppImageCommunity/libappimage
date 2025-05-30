set(
    TestDesktopIntegrationSources

    TestIntegrationManager.cpp

    integrator/TestDesktopIntegration.cpp
    integrator/TestDesktopEntryEditor.cpp

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
    PRIVATE "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated-headers>"
)

target_link_libraries(TestDesktopIntegration
    PRIVATE temporarydirectory
    PRIVATE libappimage_shared
    PRIVATE libarchive
    PRIVATE XdgUtils::DesktopEntry
    PRIVATE XdgUtils::BaseDir
    PRIVATE libsquashfuse
    # Builds using old glib versions required that the glib to be initialized, this initialization is performed
    # statically once glib is loaded.
    PRIVATE libglib
    PUBLIC dl
    PRIVATE GTest::gtest
    PRIVATE GTest::gtest_main
    PRIVATE librsvg
    PRIVATE libcairo
)

add_test(TestDesktopIntegration TestDesktopIntegration)

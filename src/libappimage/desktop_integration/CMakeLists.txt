set(
    appimage_desktop_integration_sources
    IntegrationManager.cpp
    integrator/Integrator.cpp
    integrator/DesktopEntryEditError.h
    integrator/DesktopEntryEditor.cpp
)

if(LIBAPPIMAGE_THUMBNAILER_ENABLED)
    list(APPEND appimage_desktop_integration_sources "Thumbnailer.cpp")
endif()

add_library(appimage_desktop_integration OBJECT ${appimage_desktop_integration_sources})

# Include interface directories from the libs used. Ideally we will use target_link_libraries
# but it's not supported in cmake 3.8.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html#linking-object-libraries
target_include_directories(appimage_desktop_integration
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated-headers>
)
target_link_libraries(appimage_desktop_integration
    PRIVATE libappimage_hashlib
    PRIVATE Boost::boost
    PRIVATE libarchive
    PRIVATE libsquashfuse
    PRIVATE libglib
    PRIVATE XdgUtils::DesktopEntry
    PRIVATE XdgUtils::BaseDir
)

configure_libappimage_module(appimage_desktop_integration)

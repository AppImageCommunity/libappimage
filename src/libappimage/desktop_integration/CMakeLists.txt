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
target_include_directories(
    appimage_desktop_integration
    PUBLIC $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libarchive,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libsquashfuse,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libglib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Boost::filesystem,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils::DesktopEntry,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils::BaseDir,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_libappimage_module(appimage_desktop_integration)
add_dependencies(appimage_desktop_integration
    libappimage_hashlib
    libarchive
    libsquashfuse
    Boost::filesystem
    XdgUtils::DesktopEntry
    XdgUtils::BaseDir
)

set(
    appimage_desktop_integration_sources
    IntegrationManager.cpp
    integrator/Integrator.cpp
    integrator/DesktopEntryEditError.h
    integrator/DesktopEntryEditor.cpp
    integrator/ResourcesExtractor.cpp
)

if(LIBAPPIMAGE_THUMBNAILER_ENABLED)
    list(APPEND appimage_desktop_integration_sources "Thumbnailer.cpp")
endif()

add_library(appimage_desktop_integration OBJECT ${appimage_desktop_integration_sources})


target_include_directories(
    appimage_desktop_integration
    PUBLIC $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Boost::filesystem,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libarchive,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libsquashfuse,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libglib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_libappimage_module(appimage_desktop_integration)
add_dependencies(appimage_desktop_integration libappimage_hashlib Boost::filesystem libarchive libsquashfuse XdgUtils)

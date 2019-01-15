add_library(
    appimage_desktop_integration OBJECT

    IntegrationManager.cpp
    integrator/Integrator.cpp
    integrator/DesktopIntegrationErrors.h
    integrator/DesktopEntryEditor.cpp
    integrator/ResourcesExtractor.cpp
    thumbnailer/Exceptions.h
    thumbnailer/Thumbnailer.cpp
    thumbnailer/SvgThumbnailer.cpp
)

target_include_directories(
    appimage_desktop_integration
    PUBLIC $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Boost,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libarchive,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libsquashfuse,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libglib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:CImg,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_libappimage_module(appimage_desktop_integration)
add_dependencies(appimage_desktop_integration libappimage_hashlib Boost libarchive libsquashfuse XdgUtils)

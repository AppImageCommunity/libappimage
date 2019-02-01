add_library(
    appimage_desktop_integration_integrator OBJECT

    Integrator.cpp
    DesktopEntryEditError.h
    DesktopEntryEditor.cpp
    ResourcesExtractor.cpp
)

target_include_directories(
    appimage_desktop_integration_integrator

    PRIVATE $<TARGET_PROPERTY:Boost,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

configure_libappimage_module(appimage_desktop_integration_integrator)
add_dependencies(appimage_desktop_integration_integrator core Boost libappimage_hashlib XdgUtils)

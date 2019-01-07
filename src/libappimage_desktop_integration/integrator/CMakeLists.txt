add_library(
    desktop_integration_integrator OBJECT

    Integrator.cpp
    DesktopIntegrationErrors.h
    DesktopEntryEditor.cpp
    ResourcesExtractor.cpp
)

target_include_directories(
    desktop_integration_integrator

    PUBLIC $<TARGET_PROPERTY:libappimage_0.2,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Boost,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

configure_libappimage_module(desktop_integration_integrator)

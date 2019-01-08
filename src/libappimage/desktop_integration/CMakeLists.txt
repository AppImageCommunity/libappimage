add_subdirectory(utils)
add_subdirectory(integrator)

add_library(
    appimage_desktop_integration OBJECT

    IntegrationManager.cpp
    $<TARGET_OBJECTS:appimage_desktop_integration_utils>
    $<TARGET_OBJECTS:appimage_desktop_integration_integrator>
)

target_include_directories(
    appimage_desktop_integration
    PUBLIC $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libappimage,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Boost,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils,INTERFACE_INCLUDE_DIRECTORIES>
)

configure_libappimage_module(appimage_desktop_integration)

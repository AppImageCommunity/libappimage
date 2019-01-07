add_subdirectory(utils)
add_subdirectory(integrator)

add_library(
    appimage_desktop_integration SHARED

    $<TARGET_OBJECTS:desktop_integration_utils>
    $<TARGET_OBJECTS:desktop_integration_integrator>
)

target_link_libraries(
    appimage_desktop_integration
    PUBLIC libappimage_0.2
    PRIVATE libappimage_hashlib
    PRIVATE Boost
    PRIVATE XdgUtils
)

configure_libappimage_module(appimage_desktop_integration)
